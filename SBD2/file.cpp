#include "file.h"

bool File::ReadToBuffer(int pageId)
{
	file.clear();
	if (buffer.id == pageId) //if page is already in memory do not waste resources
		return true;
	
	else if (buffer.changed) //if page wasn't changed do not waste resources
		WriteBuffer();

	buffer.changed = false;
	file.seekg(pageId * Page::BYTE_SIZE, std::ios::beg);
	if(!eof && file.read(buffer.data, Page::BYTE_SIZE))
		buffer.id = pageId;
	else
	{
		ClearBuffer();
		eof = true;
		return false;
	}
	return true;
}

void File::WriteBuffer()
{
	file.clear();

	if (!buffer.changed) //in case that someone called WriteBuffer without making changes
		return;
	
	file.seekp(buffer.id * Page::BYTE_SIZE, std::ios::beg);
	file.write(buffer.data, Page::BYTE_SIZE);

	buffer.changed = false;
}

File::File()
{
}

File::File(const std::string & fileName, int mode)
{
	Open(fileName, mode);
}

File::~File()
{
	Close();
}

void File::Open(const std::string & fileName, int mode)
{
	this->fileName = fileName;
	file.open(fileName, mode);
}

void File::Close()
{
	WriteBuffer();
	file.close();
}

void File::ClearBuffer()
{
	auto ptr = (Record*)buffer.data;
	buffer.id = -1;
	buffer.changed = false;
	for (int i = 0; i < Page::PAGE_SIZE; i++)
		ptr[i] = {0, Record::UNINIT, Record::UNINIT };
}

void File::SwitchToReadMode()
{
	WriteBuffer();
	file.close();
	file.open(fileName, DEFAULT_INPUT_MODE);
}

void File::SwitchToWriteMode()
{
	file.close();
	file.open(fileName, DEFAULT_OUTPUT_MODE);
}
