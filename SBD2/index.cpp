#include "index.h"

int Index::GetPageId(int key)
{
	if (key < 0)
		return -1;
	for(int i = 0; i < BUFFER_SIZE-1; i++)
		if (key >= bufferedIndexes[i].key && key < bufferedIndexes[i+1].key)
			return bufferedIndexes[i].pageId;
	return bufferedIndexes[BUFFER_SIZE - 1].pageId;
}
