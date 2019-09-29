#include "zipper.h"
#include "defs.h"
#include "tools.h"
#include "CDirEntry.h"

#include <fstream>
#include <stdexcept>

namespace zipper {

	struct Zipper::Impl
	{
		Zipper& m_outer;
		zipFile m_zf;
		ourmemory_t m_zipmem;
		zlib_filefunc_def m_filefunc;

		Impl(Zipper& outer) : m_outer(outer), m_zipmem(), m_filefunc()
		{
			m_zf = NULL;
			//m_filefunc = { 0 };
		}

		bool initFile(const std::string& filename)
		{
			#ifdef USEWIN32IOAPI
				zlib_filefunc64_def ffunc = { 0 };
			#endif

			int mode = 0;
			int flags = Zipper::Append;

			/* open the zip file for output */
			if (checkFileExists(filename))
				mode = (flags & Zipper::Overwrite) ? APPEND_STATUS_CREATE : APPEND_STATUS_ADDINZIP;
			else
				mode = APPEND_STATUS_CREATE;

			#ifdef USEWIN32IOAPI
				fill_win32_filefunc64A(&ffunc);
				m_zf = zipOpen2_64(filename.c_str(), mode, NULL, &ffunc);
			#else
				m_zf = zipOpen64(filename.c_str(), mode);
			#endif

			return NULL != m_zf;
		}

		bool initWithStream(std::iostream& stream)
		{
			m_zipmem.grow = 1;

			stream.seekg(0, std::ios::end);
			size_t size = (size_t)stream.tellg();
			stream.seekg(0);

			if (size > 0)
			{
				m_zipmem.base = new char[(size_t)size];
				stream.read(m_zipmem.base, size);
			}

			fill_memory_filefunc(&m_filefunc, &m_zipmem);
			
			return initMemory(size > 0 ? APPEND_STATUS_CREATE : APPEND_STATUS_ADDINZIP, m_filefunc);
		}

		bool initWithVector(std::vector<unsigned char>& buffer)
		{
			m_zipmem.grow = 1;

			if (!buffer.empty())
			{
				m_zipmem.base = new char[buffer.size()];
				memcpy(m_zipmem.base, (char*)buffer.data(), buffer.size());
				m_zipmem.size = (uLong)buffer.size();
			}

			fill_memory_filefunc(&m_filefunc, &m_zipmem);

			return initMemory(buffer.empty() ? APPEND_STATUS_CREATE : APPEND_STATUS_ADDINZIP, m_filefunc);
		}

		bool initMemory(int mode, zlib_filefunc_def& filefunc)
		{
			m_zf = zipOpen3("__notused__", mode, 0, 0, &filefunc);
			return m_zf != NULL;
		}

		bool add(std::istream& input_stream, const std::string& nameInZip, const std::string& password, int flags)
		{
			if (!m_zf) return false;

			int compressLevel = 0;
			int zip64 = 0;
			int size_buf = WRITEBUFFERSIZE;
			int err = ZIP_OK;
			unsigned long crcFile = 0;

			zip_fileinfo zi = { 0 };
			size_t size_read;

			std::vector<char> buff;
			buff.resize(size_buf);

			if (nameInZip.empty())
				return false;

			if (flags & Zipper::Faster) compressLevel = 1;
			if (flags & Zipper::Better) compressLevel = 9;

			zip64 = (int)isLargeFile(input_stream);
			if (password.empty())
				err = zipOpenNewFileInZip64(m_zf,
					nameInZip.c_str(),
					&zi,
					NULL,
					0,
					NULL,
					0,
					NULL /* comment*/,
					(compressLevel != 0) ? Z_DEFLATED : 0,
					compressLevel,
					zip64);
			else
			{
				getFileCrc(input_stream, buff, crcFile);
				err = zipOpenNewFileInZip3_64(m_zf,
					nameInZip.c_str(),
					&zi,
					NULL,
					0,
					NULL,
					0,
					NULL /* comment*/,
					(compressLevel != 0) ? Z_DEFLATED : 0,
					compressLevel,
					0,
					/* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
					-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
					password.c_str(),
					crcFile,
					zip64);
			}

			if (ZIP_OK == err)
			{
				do {
					err = ZIP_OK;
					input_stream.read(buff.data(), buff.size());
					size_read = (size_t)input_stream.gcount();
					if (size_read < buff.size() && !input_stream.eof() && !input_stream.good())
						err = ZIP_ERRNO;

					if (size_read > 0)
						err = zipWriteInFileInZip(this->m_zf, buff.data(), (unsigned int)size_read);

				} while ((err == ZIP_OK) && (size_read>0));
			}
			else
				throw EXCEPTION_CLASS(("Error adding '" + nameInZip + "' to zip").c_str());

			if (ZIP_OK == err)
				err = zipCloseFileInZip(this->m_zf);

			return ZIP_OK == err;
		}

		void close()
		{
			if (m_zf)
				zipClose(m_zf, NULL);

			if (m_zipmem.base && m_zipmem.limit > 0)
			{
				if (m_outer.m_usingMemoryVector)
				{
					m_outer.m_vecbuffer.resize(m_zipmem.limit);
					m_outer.m_vecbuffer.assign(m_zipmem.base, m_zipmem.base + m_zipmem.limit);
				}

				else if (m_outer.m_usingStream)
					m_outer.m_obuffer.write(m_zipmem.base, m_zipmem.limit);
			}

			free(m_zipmem.base);
		}
	};

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	Zipper::Zipper(const std::string& zipname)
		: m_obuffer(*(new std::stringstream())) //not used but using local variable throws exception
		, m_vecbuffer(*(new std::vector<unsigned char>())) //not used but using local variable throws exception
		, m_usingMemoryVector(false)
		, m_usingStream(false)
		, m_zipname(zipname)
		, m_impl(new Impl(*this))
	{
		if (!m_impl->initFile(zipname))
			throw EXCEPTION_CLASS("Error creating zip in file!");

		m_open = true;
	}

	Zipper::Zipper(const std::string& zipname, const std::string& password)
		: m_obuffer(*(new std::stringstream())) //not used but using local variable throws exception
		, m_vecbuffer(*(new std::vector<unsigned char>())) //not used but using local variable throws exception
		, m_usingMemoryVector(false)
		, m_usingStream(false)
		, m_zipname(zipname)
		, m_password(password)
		, m_impl(new Impl(*this))
	{
		if (!m_impl->initFile(zipname))
			throw EXCEPTION_CLASS("Error creating zip in file!");

		m_open = true;
	}

	Zipper::Zipper(std::iostream& buffer)
		: m_vecbuffer(*(new std::vector<unsigned char>())) //not used but using local variable throws exception
		, m_obuffer(buffer)
		, m_usingMemoryVector(false)
		, m_usingStream(true)
		, m_impl(new Impl(*this))
	{
		if (!m_impl->initWithStream(m_obuffer))
			throw EXCEPTION_CLASS("Error creating zip in memory!");

		m_open = true;
	}

	Zipper::Zipper(std::vector<unsigned char>& buffer)
		: m_vecbuffer(buffer)
		, m_obuffer(*(new std::stringstream())) //not used but using local variable throws exception
		, m_usingMemoryVector(true)
		, m_usingStream(false)
		, m_impl(new Impl(*this))
	{
		if (!m_impl->initWithVector(m_vecbuffer))
			throw EXCEPTION_CLASS("Error creating zip in memory!");

		m_open = true;
	}

	Zipper::~Zipper(void)
	{
		close();
	}

	bool Zipper::add(std::istream& source, const std::string& nameInZip, zipFlags flags)
	{
		return m_impl->add(source, nameInZip, "", flags);
	}

	bool Zipper::add(const std::string& fileOrFolderPath, zipFlags flags)
	{
		if (isDirectory(fileOrFolderPath))
		{
			std::string folderName = fileNameFromPath(fileOrFolderPath);
			std::vector<std::string> files = filesFromDirectory(fileOrFolderPath);
			std::vector<std::string>::iterator it = files.begin();
			for (; it != files.end(); ++it)
			{
				std::ifstream input(it->c_str(), std::ios::binary);
				std::string nameInZip = it->substr(it->rfind(folderName + CDirEntry::Separator), it->size());
				add(input, nameInZip, flags);
				input.close();
			}
		}
		else
		{
			std::ifstream input(fileOrFolderPath.c_str(), std::ios::binary);
			add(input, fileNameFromPath(fileOrFolderPath), flags);
			input.close();
		}

		return true;
	}


	void Zipper::open()
	{
		if (!m_open)
		{
			if (m_usingMemoryVector)
			{
				if (!m_impl->initWithVector(m_vecbuffer))
					throw EXCEPTION_CLASS("Error opening zip memory!");
			}
			else if (m_usingStream)
			{
				if (!m_impl->initWithStream(m_obuffer))
					throw EXCEPTION_CLASS("Error opening zip memory!");
			}
			else
			{
				if (!m_impl->initFile(m_zipname))
					throw EXCEPTION_CLASS("Error opening zip file!");
			}

			m_open = true;
		}
	}

	void Zipper::close()
	{
		if (m_open)
		{
			m_impl->close();
			m_open = false;
		}
	}
}
