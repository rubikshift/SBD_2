#pragma once
#include "index.h"

struct DataBase
{
	DataBase();
	void Open(std::string indexFileName, std::string dbFileName, std::string metadataFileName ,bool clear, int* IOCounter, int reservedPages = 1, bool tmp = false);
	void Insert(Record r);
	bool Delete(int key);
	Record Get(int key);
	Record GetNext();
	void Reorganize();
	void Info();

	void GenerateMetadata(std::string fileName);
	void ReadMetadata(std::string fileName);


	void Print();

	File file;
	Index index;
	Record* SearchMainArea(int key);

	static constexpr const double o = 0.2; // overflowAreaSize = ceil(mainAreaSize * o)
	static constexpr const double alfa = 0.5;

	int mainAreaCount, overflowAreaCount;

	int overflowPtr, overflowStart, overflowEnd, mainAreaSize;
};