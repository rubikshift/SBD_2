#pragma once
#include "index.h"

struct DataBase
{
	DataBase();
	void Open(std::string indexFileName, std::string dbFileName, bool clear, int reservedPages = 1, bool tmp = false);
	void Insert(Record r);
	bool Delete(int key);
	Record Get(int key);
	//Record GetNext();
	void Reorganize();

	void Print();

	File file;
	Index index;
	Record* FindRecord(int key);

	static constexpr const double o = 0.2; // overflowAreaSize = ceil(mainAreaSize * o)
	static constexpr const double alfa = 0.5;

	int mainAreaCount, overflowAreaCount;

	int overflowPtr, overflowStart, overflowEnd, mainAreaSize;
};