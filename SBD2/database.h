#pragma once
#include "index.h"
class DataBase
{
	public:
		DataBase();
		DataBase(std::string indexFileName, std::string dbFileName);
		void Add(const Record& r);
		void Update(const Record& r);
		void Delete(int key);
		Record Get(int key);
		void Reorganize();

	private:
		double alfa, beta; //alfa wspolczynnik zajetosci strony, beta prog reorganizacji
		File file;
		Index index;
};

std::ostream& operator<<(std::ostream&, DataBase& db);