#include "database.h"

void DataBase::Insert(const Record & r)
{
	if (r.key < 0) // can not insert deleted record
		return;

	auto pageId = index.GetPageId(r.key);
	file.ReadToBuffer(pageId);
	auto record = FindRecord(r.key);
	
	//UPDATE RECORD
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
	}

	//INSERT NEW RECORD (overflow area)
	else
	{
		//update record pointer
		record->ptr = overflowPtr;
		file.buffer.changed = true;

		file.ReadToBuffer(overflowPtr % Page::BYTE_SIZE);
		record = ((Record*)file.buffer.data) + overflowPtr / Page::BYTE_SIZE;

		record->key = r.key;
		record->v = r.v;
		record->m = r.m;
	}

	file.buffer.changed = true;
	file.WriteBuffer();
	return;
}

void DataBase::Delete(int key)
{
	auto pageId = index.GetPageId(key);
	file.ReadToBuffer(pageId);
	auto record = FindRecord(key);
	if (!record->isInitialized())
		return;
	record->key = -abs(record->key);
	file.buffer.changed = true;
	file.WriteBuffer();
}

Record DataBase::Get(int key)
{
	auto pageId = index.GetPageId(key);
	file.ReadToBuffer(pageId);
	auto record = FindRecord(key);
	return *record;
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
		else if (!ptr[i].isInitialized())
			record = ptr + i;

		if (record != nullptr)
			break;
	}

	int pageId, offset;
	while (!record->isInitialized() || abs(record->key) != key || record->ptr == Record::NO_PTR)
	{
		pageId = record->ptr % Page::PAGE_SIZE;
		offset = record->ptr / Page::PAGE_SIZE;
		file.ReadToBuffer(pageId);
		record = ptr + offset;
	}
	
	return record;
}
