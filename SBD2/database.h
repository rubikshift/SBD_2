#pragma once
#include "index.h"
class DataBase
{
	public:
		DataBase(std::string indexFileName, std::string dbFileName, bool clear);
		void Insert(const Record& r);
		void Delete(int key);
		Record Get(int key);
		Record GetNext();
		void Reorganize();

	private:
		File file;
		Index index;
		Record* FindRecord(int key);
};

std::ostream& operator<<(std::ostream&, DataBase& db);