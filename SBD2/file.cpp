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
	if (file.read(buffer.data, Page::BYTE_SIZE))
	{
		buffer.id = pageId;
		(*IOCounter)++;
	}
	else
	{
		ClearBuffer();
		return false;
	}
	return true;
}

void File::WriteBuffer()
{
	file.clear();

	if (!buffer.changed || buffer.id == -1) //in case that someone called WriteBuffer without making changes
		return;
	
	file.seekp(buffer.id * Page::BYTE_SIZE, std::ios::beg);
	file.write(buffer.data, Page::BYTE_SIZE);
	(*IOCounter)++;

	buffer.changed = false;
}

File::File()
{
}

File::File(const std::string & fileName, int* IOCounter, int mode)
{
	Open(fileName, IOCounter, mode);
}

File::~File()
{
	Close();
}

void File::Open(const std::string & fileName, int* IOCounter, int mode)
{
	buffer.id = -1;
	this->fileName = fileName;
	this->IOCounter = IOCounter;
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
		ptr[i] = {-1, Record::UNINIT, Record::UNINIT };
}

void File::CreateSpace(int pages)
{
	file.seekp(std::ios::beg);
	for (int i = 0; i < pages; i++)
	{
		ClearBuffer();
		buffer.changed = true;
		buffer.id = i;
		WriteBuffer();
	}
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
