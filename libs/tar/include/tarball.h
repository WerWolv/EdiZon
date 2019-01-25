/*
 * tarball.h
 *
 *  Created on: Jul 28, 2010
 *      Author: Pierre Lindenbaum PhD
 *              plindenbaum@yahoo.fr
 *              http://plindenbaum.blogspot.com
 *              
 */

#ifndef LINDENB_IO_TARBALL_H_
#define LINDENB_IO_TARBALL_H_

#include <iostream>


namespace lindenb { namespace io {

/**
 *  A Tar Archive
 */
class Tar
    {
    private:
	bool _finished;
    protected:
	std::ostream& out;
	void _init(void* header);
	void _checksum(void* header);
	void _size(void* header,unsigned long fileSize);
	void _filename(void* header,const char* filename);
	void _endRecord(std::size_t len);
    public:
	Tar(std::ostream& out);
	virtual ~Tar();
	/** writes 2 empty blocks. Should be always called before closing the Tar file */
	void finish();
	void put(const char* filename,const std::string& s);
	void put(const char* filename,const char* content);
	void put(const char* filename,const char* content,std::size_t len);
	void putFile(const char* filename,const char* nameInArchive);
    };


}}

#endif /* LINDENB_IO_TARBALL_H_ */