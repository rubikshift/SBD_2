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
		Record FindRecord(Page& page, int key); //Locates and returns record on page
		void AddRecord(Page& page, const Record& record);
		Page FollowPtrs(Page& page, int key); //Finds page with record or page where records should be added
};

std::ostream& operator<<(std::ostream&, DataBase& db);