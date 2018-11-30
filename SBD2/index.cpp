#include <cstring>
#include "index.h"

Index::Index() : file(), pos(0)
{
	ClearBufferedIndexes();
}

Index::~Index()
{
	file.Close();
}



int Index::GetPageId(int key)
{
	for (unsigned int i = 0; i < BUFFER_SIZE - 1; i++)
		if (bufferedIndexes[i].key <= key &&
			(bufferedIndexes[i + 1].key >= key || bufferedIndexes[i+1].key == IndexRecord::UNINIT))
			return bufferedIndexes[i].pageId;
	return bufferedIndexes[BUFFER_SIZE - 1].pageId;
}

void Index::Open(std::string fileName, bool clear)
{
	auto mode = clear ? File::DEFAULT_OUTPUT_MODE : File::DEFAULT_INPUT_MODE;
	file.Open(fileName, mode);

	int pageId = 0;
	auto elementsPerPage = Page::BYTE_SIZE / IndexRecord::RECORD_SIZE;
	while (!clear && pageId < BUFFERED_PAGE_COUNT && file.ReadToBuffer(pageId)) // LOAD DATA
	{
		auto ptr = (IndexRecord*)file.buffer.data;
		for (unsigned int i = 0; i < elementsPerPage; i++)
		{
			if (!ptr[i].isInitialized())
				break;

			bufferedIndexes[pageId * elementsPerPage + i] = ptr[i];
		}
		pageId++;
	}
}

void Index::ClearBufferedIndexes()
{
	for (unsigned int i = 0; i < BUFFER_SIZE; i++)
		bufferedIndexes[i] = { IndexRecord::UNINIT, IndexRecord::UNINIT };
	pos = 0;
}

void Index::AddIndexRecord(const IndexRecord & indexrecord)
{
	if (pos < BUFFER_SIZE)
		bufferedIndexes[pos] = indexrecord;
	pos++;
}

void Index::Save()
{
	file.SwitchToWriteMode();
	auto ptr = (IndexRecord*)file.buffer.data;
	int pageId = 0;
	pos = 0;
	auto elementsPerPage = Page::BYTE_SIZE / IndexRecord::RECORD_SIZE;
	while (bufferedIndexes[pos].isInitialized())
	{
		file.buffer.id = pageId;
		for (unsigned int i = 0; i < elementsPerPage; i++)
			ptr[i] = bufferedIndexes[pos + i];
		file.buffer.changed = true;
		file.WriteBuffer();

		pos += elementsPerPage;
	}

	file.SwitchToReadMode();
}

std::ostream & operator<<(std::ostream & os, const Index & index)
{
	for (int i = 0; i < Index::BUFFER_SIZE; i++)
	{
		if (!index.bufferedIndexes[i].isInitialized())
			break;

		std::cout << index.bufferedIndexes[i].key << "->" << index.bufferedIndexes[i].pageId << "\n";
	}
	return os;
}

IndexRecord::IndexRecord(int key, int pageId) : key(key), pageId(pageId)
{
}

bool IndexRecord::isInitialized() const
{
	return key != UNINIT;
}
