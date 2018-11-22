#include "database.h"

void DataBase::Insert(const Record & r)
{
	if (r.key < 0) // can not insert deleted record
		return;
	auto pageId = index.GetPageId(r.key);
	file.ReadToBuffer(pageId);
	auto record = FindRecord(r.key);
	
	//UPDATE RECORD
	if (record->isInitialized() && r.key > 0) // r.key > 0 <=> can not update guard
	{
		record->key = r.key; //if record was tagged as deleted, remove tag
		record->v = r.v;
		record->m = r.m;
		file.buffer.changed = true;
	}

	//INSERT NEW RECORD
	else
	{

	}

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

Record * DataBase::FindRecord(int key)
{
	return nullptr;
}
