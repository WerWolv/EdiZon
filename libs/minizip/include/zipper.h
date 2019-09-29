#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

namespace zipper {

	class Zipper
	{
	public:
		// Minizip options/params:
		//              -o                -a             -0            -1             -9             -j
		enum zipFlags { Overwrite = 0x01, Append = 0x02, Store = 0x04, Faster = 0x08, Better = 0x10, NoPaths = 0x20 };

		Zipper(std::iostream& buffer);
		Zipper(std::vector<unsigned char>& buffer);
		Zipper(const std::string& zipname);
		Zipper(const std::string& zipname, const std::string& password);

		~Zipper(void);

		bool add(std::istream& source, const std::string& nameInZip = std::string(), zipFlags flags = Better);
		bool add(const std::string& fileOrFolderPath, zipFlags flags = Better);
		
		void open();
		void close();

	private:
		std::string m_password;
		std::string m_zipname;
		std::iostream& m_obuffer;
		std::vector<unsigned char>& m_vecbuffer;
		bool m_usingMemoryVector;
		bool m_usingStream;
		bool m_open;

		struct Impl;
		Impl* m_impl;
	};
}
