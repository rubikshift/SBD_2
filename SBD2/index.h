#pragma once
#include "file.h"

struct IndexRecord
{
	int key, pageId;
	static const unsigned int RECORD_SIZE = sizeof(key) + sizeof()
};

class Index
{
	public:
		Index();
		int GetPageId(int key);

		static const unsigned int BUFFERED_PAGE_COUNT = 5;
		static const unsigned int BUFFER_SIZE = Page::BYTE_SIZE / (2*sizeof(int)) * BUFFERED_PAGE_COUNT;

	private:
		int keys[BUFFER_SIZE/2], pageIds[BUFFER_SIZE/2];

};
