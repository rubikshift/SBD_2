#include "database.h"

void DataBase::Add(const Record & r)
{
	auto pageId = index.GetPageId(r.key);
	auto page = file.ReadPage(pageId);
	page = FollowPtrs(page, r.key);
	auto record = FindRecord(page, r.key);
	if (record.isInitialized())
		return; // zaktualizuj rekord
	AddRecord(page, r);
	file.WritePage(page);
}

void DataBase::Update(const Record & r)
{
	auto pageId = index.GetPageId(r.key);
	auto page = file.ReadPage(pageId);

	file.WritePage(page);
}

void DataBase::Delete(int key)
{
	auto pageId = index.GetPageId(key);
	auto page = file.ReadPage(pageId);
	auto record = FindRecord(page, key);
	if (!record.isInitialized())
		return;
	record.key = -abs(record.key);
	//zapisz strone ze zmodyfikowanym rekordem

	file.WritePage(page);
}

Record DataBase::Get(int key)
{
	auto pageId = index.GetPageId(key);
	auto page = file.ReadPage(pageId);
	return FindRecord(page, key);
}
