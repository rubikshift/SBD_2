#include <cstdio>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include "database.h"

DataBase::DataBase() : 
	file(), 
	index(), 
	overflowPtr(Record::NO_PTR), 
	overflowStart(Record::NO_PTR), 
	overflowEnd(Record::NO_PTR),
	primaryAreaCount(0),
	overflowAreaCount(0)
{
}

void DataBase::Open(std::string indexFileName, std::string dbFileName, std::string metadataFileName, bool clear, int* IOCounter, int reservedPages, bool tmp)
{
	
	index.Open(indexFileName, IOCounter, clear);

	//CREATE NEW FILE
	if (clear)
	{
		file.Open(dbFileName, IOCounter, File::DEFAULT_OUTPUT_MODE);
		file.Close();
	}
	
	file.Open(dbFileName, IOCounter, File::DEFAULT_INPUT_OUTPUT_MODE);
	
	//INIT FILE
	if (clear)
	{
		primaryAreaSize = reservedPages;
		int overflowAreaSize = static_cast<int>(std::ceil(primaryAreaSize * o));
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
	auto record = SearchMainArea(r.key);
	
	//UPDATE RECORD MAIN AREA
	if (record->isInitialized() && r.key > 0 && abs(record->key) == r.key) // r.key > 0 <=> can not update guard
	{
		if (record->key < 0)
			primaryAreaCount++;
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
		primaryAreaCount++;
	}

	//UPDATE OR INSERT NEW RECORD (overflow area)
	else if(r.key > 0) // r.key > 0 <=> can not insert new guard
	{
		int offset;
		bool changedChain = false;

		//FOLLOW PTRS
		while (record->isInitialized() && abs(record->key) != r.key && record->ptr != Record::NO_PTR && !changedChain)
		{
			
			if (abs(record->key) > r.key)
			{
				r.ptr = record->ptr;
				std::swap(record->key, r.key);
				std::swap(record->key, r.key);
				std::swap(record->key, r.key);
				changedChain = true;
				break;
			}

			pageId = record->ptr / Page::BYTE_SIZE;
			offset = (record->ptr % Page::BYTE_SIZE)/Record::RECORD_SIZE;
			file.ReadToBuffer(pageId);
			record = ptr + offset;
		}
		
		if (record->isInitialized() && abs(record->key) != r.key)
		{
			record->ptr = overflowPtr;
			file.buffer.changed = true;
			file.ReadToBuffer(overflowPtr / Page::BYTE_SIZE);
			record = ptr + (overflowPtr % Page::BYTE_SIZE) / Record::RECORD_SIZE;
			overflowPtr += Record::RECORD_SIZE;
			overflowAreaCount++;
		}
		if (record->isInitialized() && record->key < 0)
			overflowAreaCount++;
		record->key = r.key;
		record->v = r.v;
		record->m = r.m;
		record->ptr = r.ptr;
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
	auto record = SearchMainArea(key);

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
		primaryAreaCount--;
	else
		overflowAreaCount--;

	record->key = -abs(record->key);
	file.buffer.changed = true;
	file.WriteBuffer();

	return true;
}

Record DataBase::Get(int key)
{
	auto ptr = (Record*)file.buffer.data;
	auto pageId = index.GetPageId(key);
	file.ReadToBuffer(pageId);
	auto record = SearchMainArea(key);

	int offset = 0;
	while (record->isInitialized() && abs(record->key) != key && record->ptr != Record::NO_PTR)
	{
		pageId = record->ptr / Page::BYTE_SIZE;
		offset = (record->ptr % Page::BYTE_SIZE) / Record::RECORD_SIZE;
		file.ReadToBuffer(pageId);
		record = ptr + offset;
	}

	return *record;
}

Record DataBase::GetNext()
{
	static int offset = 0, pageId = 0, overflowPageId = 0, overflowOffset = 0;
	static bool readOverflow = false;

	auto ptr = (Record*)file.buffer.data;
	Record record;

	if (pageId == primaryAreaSize)
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
	
	std::cout << "\nREORGANIZE - START, IOCounter: " << *file.IOCounter << std::endl;

	auto newSize = static_cast<int>(std::ceil((primaryAreaCount + overflowAreaCount) / (Page::PAGE_SIZE * alfa)));
	tmp.Open(index.file.fileName + "_tmp", file.fileName + "_tmp", file.fileName + "_meta" ,true, file.IOCounter, newSize, true);

	std::cout << "REORGANIZE - CREATED NEW DB, IOCounter: " << *file.IOCounter << std::endl;

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
	std::cout << "REORGANIZE - COPIED OLD DB, IOCounter: " << *file.IOCounter << std::endl;
	tmp.primaryAreaCount = primaryAreaCount + overflowAreaCount;
	tmp.index.Save();
	std::cout << "REORGANIZE - SAVED INDEX, IOCounter: " << *file.IOCounter << std::endl;
	tmp.GenerateMetadata(file.fileName + "_meta");
	std::cout << "REORGANIZE - GENERATED METADATA, IOCounter: " << *file.IOCounter << std::endl;

	tmp.file.Close();
	tmp.index.file.Close();
	this->file.Close();
	this->index.file.Close();

	std::remove(index.file.fileName.c_str());
	std::remove(file.fileName.c_str());
	std::rename(tmp.index.file.fileName.c_str(), index.file.fileName.c_str());
	std::rename(tmp.file.fileName.c_str(), file.fileName.c_str());

	Open(index.file.fileName, file.fileName, file.fileName + "_meta", false, file.IOCounter);
	std::cout << "REORGANIZE - RENAMED AND REOPENED DB, IOCounter: " << *file.IOCounter << std::endl;
}

void DataBase::Info()
{
	std::cout << "\nName: " << file.fileName << "\n"
		<< "Primary area size: " << primaryAreaSize << "\n"
		<< "Overflow area size: " << (overflowEnd - overflowStart) / Page::BYTE_SIZE << "\n"
		<< "Overflow free ptr: " << overflowPtr << "\n"
		<< "Records in main area: " << primaryAreaCount << "\n"
		<< "Records in overflow: " << overflowAreaCount << "\n"
		<< "BufferPageId: " << file.buffer.id << std::endl;
}

void DataBase::GenerateMetadata(std::string fileName)
{
	File metadata(fileName, file.IOCounter, File::DEFAULT_OUTPUT_MODE);
	auto ptr = (int*)metadata.buffer.data;
	metadata.buffer.id = 0;
	metadata.buffer.changed = true;

	ptr[0] = primaryAreaCount;
	ptr[1] = overflowAreaCount;
	ptr[2] = overflowPtr;
	ptr[3] = overflowStart;
	ptr[4] = overflowEnd;
	ptr[5] = primaryAreaSize;

	metadata.WriteBuffer();
}

void DataBase::ReadMetadata(std::string fileName)
{
	File metadata(fileName, file.IOCounter, File::DEFAULT_INPUT_MODE);
	metadata.ReadToBuffer(0);
	auto ptr = (int*)metadata.buffer.data;

	primaryAreaCount = ptr[0];
	overflowAreaCount = ptr[1];
	overflowPtr = ptr[2];
	overflowStart = ptr[3];
	overflowEnd = ptr[4];
	primaryAreaSize = ptr[5];
}

void DataBase::Print()
{
	int pageId = 0;
	auto ptr = (Record*)file.buffer.data;

	int overflowFirstPage = overflowStart / Page::BYTE_SIZE;

	std::cout << index << std::endl;
	std::cout << "File: " << std::endl;
	while (file.ReadToBuffer(pageId))
	{
		std::cout << "\tPAGE " << std::setw(5) << std::left << pageId;
		if (pageId < overflowFirstPage)
			 std::cout << std::setw(70) << std::setfill('#') << std::right << " PRIMARY";
		else
			std::cout << std::setw(70) << std::setfill('#') << std::right << " OVERFLOW";
		std::cout << "" << std::setfill(' ') << std::endl;
		
		for (int i = 0; i < Page::PAGE_SIZE; i++)
			std::cout << "\t\t" << std::setw(5) << std::right << i << ". " << ptr[i] << std::endl;
		pageId++;
	}
}

Record * DataBase::SearchMainArea(int key)
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
