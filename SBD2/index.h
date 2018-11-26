#pragma once
#include <string>
#include "file.h"

struct IndexRecord
{
	IndexRecord(int key = UNINIT, int pageId = UNINIT);

	int key, pageId;
	static const unsigned int RECORD_SIZE = sizeof(key) + sizeof(pageId);
	static const int UNINIT = -1;

	bool isInitialized() const;
};

struct Index
{
	Index();
	~Index();

	int GetPageId(int key);
	void Open(std::string fileName, bool clear);

	void ClearBufferedIndexes();
	void AddIndexRecord(const IndexRecord& indexrecord);
	void Save();

	static const unsigned int BUFFERED_PAGE_COUNT = 5;
	static const unsigned int BUFFER_SIZE = Page::BYTE_SIZE / IndexRecord::RECORD_SIZE * BUFFERED_PAGE_COUNT;

	IndexRecord bufferedIndexes[BUFFER_SIZE];
	File file;

	unsigned int pos;
};

std::ostream& operator<<(std::ostream& os, const Index& index);
