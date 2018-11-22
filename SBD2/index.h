#pragma once
#include "file.h"

struct IndexRecord
{
	int key, pageId;
	static const unsigned int RECORD_SIZE = sizeof(key) + sizeof(pageId);
};

class Index
{
	public:
		Index();
		int GetPageId(int key);

		static const unsigned int BUFFERED_PAGE_COUNT = 5;
		static const unsigned int BUFFER_SIZE = Page::BYTE_SIZE / IndexRecord::RECORD_SIZE * BUFFERED_PAGE_COUNT;

	private:
		IndexRecord bufferedIndexes[BUFFER_SIZE];
		File file;

};
