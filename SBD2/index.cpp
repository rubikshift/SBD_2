#include "index.h"

Index::Index(std::string fileName, bool clear) : file()
{
	auto mode = clear ? File::DEFAULT_OUTPUT_MODE : File::DEFAULT_INPUT_MODE;
	file.Open(fileName, mode);
	
	int pageId = 0;
	auto elementsPerPage = Page::BYTE_SIZE / IndexRecord::RECORD_SIZE;
	while(!clear && pageId < BUFFERED_PAGE_COUNT && file.ReadToBuffer(pageId)) // LOAD DATA
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

int Index::GetPageId(int key)
{
	return -1;
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

IndexRecord::IndexRecord() : key(UNINIT), pageId(UNINIT)
{
}

bool IndexRecord::isInitialized() const
{
	return key != UNINIT;
}
