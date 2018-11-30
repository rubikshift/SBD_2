#include <cstdio>
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

void DataBase::Open(std::string indexFileName, std::string dbFileName, std::string metadataFileName, bool clear, int reservedPages, bool tmp)
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
	else
		ReadMetadata(metadataFileName);
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
	if (key <= 0)
		return false;
	
	bool mainArea = true;
	auto ptr = (Record*)file.buffer.data;
	auto pageId = index.GetPageId(key);
	file.ReadToBuffer(pageId);
	auto record = FindRecord(key);

	int offset = 0;
	while (record->isInitialized() && abs(record->key) != key && record->ptr != Record::NO_PTR)
	{
		mainArea = false;
		pageId = record->ptr / Page::BYTE_SIZE;
		offset = (record->ptr % Page::BYTE_SIZE) / Record::RECORD_SIZE;
		file.ReadToBuffer(pageId);
		record = ptr + offset;
	}

	if (!record->isInitialized() || abs(record->key) != key)
		return false;

	if (mainArea)
		mainAreaCount--;
	else
		overflowAreaCount--;

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

Record DataBase::GetNext()
{
	static int offset = 0, pageId = 0, overflowPageId = 0, overflowOffset = 0;
	static bool readOverflow = false;

	auto ptr = (Record*)file.buffer.data;
	Record record;

	if (pageId == mainAreaSize)
	{
		offset = pageId = overflowPageId = overflowOffset = 0;
		readOverflow = false;
		return { 0, Record::UNINIT, Record::UNINIT };
	}

	if (readOverflow)
	{
		file.ReadToBuffer(overflowPageId);
		record = ptr[overflowOffset];

		if (ptr[overflowOffset].ptr == Record::NO_PTR)
			readOverflow = false;
		else
		{
			overflowPageId = ptr[overflowOffset].ptr / Page::BYTE_SIZE;
			overflowOffset = (ptr[overflowOffset].ptr % Page::BYTE_SIZE) / Record::RECORD_SIZE;
		}
	}
	else
	{
		file.ReadToBuffer(pageId);
		record = ptr[offset];

		if(ptr[offset].ptr != Record::NO_PTR)
		{
			readOverflow = true;
			overflowPageId = ptr[offset].ptr / Page::BYTE_SIZE;
			overflowOffset = (ptr[offset].ptr % Page::BYTE_SIZE) / Record::RECORD_SIZE;
		}

		offset = (offset + 1) % Page::PAGE_SIZE;
		if (offset == 0 || !ptr[offset].isInitialized())
		{
			offset = 0;
			pageId++;
		}
	}

	return record;
}

void DataBase::Reorganize()
{
	DataBase tmp;
	
	auto newSize = static_cast<int>(std::ceil((mainAreaCount + overflowAreaCount) / (Page::PAGE_SIZE * alfa)));
	tmp.Open(index.file.fileName + "_tmp", file.fileName + "_tmp", file.fileName + "_meta" ,true, newSize, true);

	auto ptr = (Record*)tmp.file.buffer.data;
	int pageId = 0;
	Record record = GetNext();
	unsigned int q = 0;

	while (record.isInitialized())
	{
		if (record.key < 0)
		{
			record = GetNext();
			continue;
		}

		if (q == 0)
		{
			tmp.file.ReadToBuffer(pageId);
			tmp.index.AddIndexRecord({ record.key, pageId });
		}
		
		record.ptr = Record::NO_PTR;
		tmp.file.buffer.changed = true;

		ptr[q] = record;
		q = (q + 1) % static_cast<int>((Page::PAGE_SIZE * alfa));
		if (q == 0)
			pageId++;

		record = GetNext();
	}
	tmp.mainAreaCount = mainAreaCount + overflowAreaCount;
	tmp.index.Save();
	tmp.GenerateMetadata(file.fileName + "_meta");

	tmp.file.Close();
	tmp.index.file.Close();
	this->file.Close();
	this->index.file.Close();

	std::remove(index.file.fileName.c_str());
	std::remove(file.fileName.c_str());
	std::rename(tmp.index.file.fileName.c_str(), index.file.fileName.c_str());
	std::rename(tmp.file.fileName.c_str(), file.fileName.c_str());

	Open(index.file.fileName, file.fileName, file.fileName + "_meta", false);
}

void DataBase::GenerateMetadata(std::string fileName)
{
	File metadata(fileName, File::DEFAULT_OUTPUT_MODE);
	auto ptr = (int*)metadata.buffer.data;
	metadata.buffer.id = 0;
	metadata.buffer.changed = true;

	ptr[0] = mainAreaCount;
	ptr[1] = overflowAreaCount;
	ptr[2] = overflowPtr;
	ptr[3] = overflowStart;
	ptr[4] = overflowEnd;
	ptr[5] = mainAreaSize;

	metadata.WriteBuffer();
}

void DataBase::ReadMetadata(std::string fileName)
{
	File metadata(fileName, File::DEFAULT_INPUT_MODE);
	metadata.ReadToBuffer(0);
	auto ptr = (int*)metadata.buffer.data;

	mainAreaCount = ptr[0];
	overflowAreaCount = ptr[1];
	overflowPtr = ptr[2];
	overflowStart = ptr[3];
	overflowEnd = ptr[4];
	mainAreaSize = ptr[5];
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
