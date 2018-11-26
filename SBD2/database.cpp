#include <cmath>
#include <algorithm>
#include "database.h"

DataBase::DataBase() : 
	file(), 
	index(), 
	overflowPtr(Record::NO_PTR), 
	overflowStart(Record::NO_PTR), 
	overflowEnd(Record::NO_PTR),
	mainAreaCount(0),
	overflowAreaCount(0)
{
}

void DataBase::Open(std::string indexFileName, std::string dbFileName, bool clear, int reservedPages, bool tmp)
{
	
	index.Open(indexFileName, clear);

	//CREATE NEW FILE
	if (clear)
	{
		file.Open(dbFileName, File::DEFAULT_OUTPUT_MODE);
		file.Close();
	}
	
	file.Open(dbFileName, File::DEFAULT_INPUT_OUTPUT_MODE);
	
	//INIT FILE
	if (clear)
	{
		mainAreaSize = reservedPages;
		int overflowAreaSize = static_cast<int>(std::ceil(mainAreaSize * o));
		file.CreateSpace(reservedPages + overflowAreaSize);
		overflowPtr = overflowStart = reservedPages * Page::BYTE_SIZE;
		overflowEnd = (reservedPages + overflowAreaSize) * Page::BYTE_SIZE;
		if (!tmp) {
			index.AddIndexRecord({ 0, 0 });
			index.Save();
			Insert({ 0, 0, 0 });
		}
	}
}

void DataBase::Insert(Record r)
{
	if (r.key < 0) // can not insert deleted record
		return;
	
	auto ptr = (Record*)file.buffer.data;
	auto pageId = index.GetPageId(r.key);
	file.ReadToBuffer(pageId);
	auto record = FindRecord(r.key);
	
	//UPDATE RECORD MAIN AREA
	if (record->isInitialized() && r.key > 0 && abs(record->key) == r.key) // r.key > 0 <=> can not update guard
	{
		record->key = r.key; //if record was tagged as deleted, remove tag
		record->v = r.v;
		record->m = r.m;
	}

	//INSERT NEW RECORD (main area)
	else if(!record->isInitialized())
	{
		record->key = r.key;
		record->v = r.v;
		record->m = r.m;
		mainAreaCount++;
	}

	//UPDATE OR INSERT NEW RECORD (overflow area)
	else if(r.key > 0) // r.key > 0 <=> can not insert new guard
	{
		int offset;

		//FOLLOW PTRS
		while (record->isInitialized() && abs(record->key) != r.key && record->ptr != Record::NO_PTR)
		{
			if (r.key < abs(record->key))
			{
				std::swap(record->key, r.key);
				std::swap(record->m, r.m);
				std::swap(record->v, r.v);
				file.buffer.changed = true;
			}
			pageId = record->ptr / Page::BYTE_SIZE;
			offset = (record->ptr % Page::BYTE_SIZE)/Record::RECORD_SIZE;
			file.ReadToBuffer(pageId);
			record = ptr + offset;
		}
		
		if (record->isInitialized() && abs(record->key) != r.key)
		{
			if (r.key < abs(record->key))
			{
				std::swap(record->key, r.key);
				std::swap(record->m, r.m);
				std::swap(record->v, r.v);
			}
			record->ptr = overflowPtr;
			file.buffer.changed = true;
			file.ReadToBuffer(overflowPtr / Page::BYTE_SIZE);
			record = ptr + (overflowPtr % Page::BYTE_SIZE) / Record::RECORD_SIZE;
			overflowPtr += Record::RECORD_SIZE;
			overflowAreaCount++;
		}

		record->key = r.key;
		record->v = r.v;
		record->m = r.m;
		file.buffer.changed = true;
	}

	file.buffer.changed = true;
	file.WriteBuffer();

	if (overflowEnd == overflowPtr)
		Reorganize();
	return;
}

bool DataBase::Delete(int key)
{
	auto pageId = index.GetPageId(key);
	file.ReadToBuffer(pageId);
	auto record = FindRecord(key);
	if (!record->isInitialized() || record->key != key)
		return false;
	record->key = -abs(record->key);
	file.buffer.changed = true;
	file.WriteBuffer();
	return true;
}

Record DataBase::Get(int key)
{
	auto pageId = index.GetPageId(key);
	file.ReadToBuffer(pageId);
	auto record = FindRecord(key);
	return *record;
}

void DataBase::Reorganize()
{
	DataBase tmp;
	
	auto newSize = static_cast<int>(std::ceil((mainAreaCount + overflowAreaCount) / (Page::PAGE_SIZE * alfa)));
	tmp.Open(index.file.fileName + "_tmp", file.fileName + "_tmp", true, newSize, true);

	int pageId = 0, i = 0, q = 0, bckOverflowPtr = Record::NO_PTR;
	auto ptr = (Record*)file.buffer.data;

	//while (pageId < mainAreaSize && file.ReadToBuffer(pageId))
	//{
	//	if (!ptr[i].isInitialized())
	//	{
	//		pageId++;
	//		i = 0;
	//		continue;
	//	}
	//	
	//	if (i == 0)
	//		index.AddIndexRecord({ ptr[i].key, q });
	//	bckOverflowPtr = ptr[i].ptr;
	//	ptr[i].ptr = Record::NO_PTR;


	//}
}

void DataBase::Print()
{
	int pageId = 0;
	auto ptr = (Record*)file.buffer.data;

	std::cout << index << std::endl;

	while (file.ReadToBuffer(pageId))
	{
		std::cout << pageId << std::endl;
		for (int i = 0; i < Page::PAGE_SIZE; i++)
			std::cout << "\t" << ptr[i] << std::endl;
		pageId++;
	}

	std::cout << std::endl;
}

Record * DataBase::FindRecord(int key)
{
	auto ptr = (Record*)file.buffer.data;
	Record* record = nullptr;
	
	for (int i = 0; i < Page::PAGE_SIZE; i++)
	{
		if (i == Page::BYTE_SIZE)
			record = ptr + Page::PAGE_SIZE - 1;
		else if (abs(ptr[i].key) <= key && ptr[i + 1].isInitialized() && abs(ptr[i + 1].key) > key)
			record = ptr + i;
		else if (ptr[i].isInitialized() && abs(ptr[i].key) == key)
			record = ptr + i;
		else if (!ptr[i].isInitialized())
			record = ptr + i;

		if (record != nullptr)
			break;
	}
	
	return record;
}
