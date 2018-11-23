#pragma once
#include <string>
#include "file.h"

struct IndexRecord
{
	IndexRecord();

	int key, pageId;
	static const unsigned int RECORD_SIZE = sizeof(key) + sizeof(pageId);
	static const int UNINIT = -1;

	bool isInitialized() const;
};

struct Index
{
	Index(std::string fileName, bool clear = true);
	int GetPageId(int key);

	static const unsigned int BUFFERED_PAGE_COUNT = 5;
	static const unsigned int BUFFER_SIZE = Page::BYTE_SIZE / IndexRecord::RECORD_SIZE * BUFFERED_PAGE_COUNT;

	IndexRecord bufferedIndexes[BUFFER_SIZE];
	File file;
};

std::ostream& operator<<(std::ostream& os, const Index& index);
