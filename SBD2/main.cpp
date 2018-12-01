#include <iostream>
#include <limits>
#include <ctime>
#include "database.h"

void help()
{
	std::cout << "HELP:\n"
		<< "\tinsert key v m\n"
		<< "\t\tshort: i\n"
		<< "\t\tif key exists updates record, otherwise adds record\n"
		<< "\tget key\n"
		<< "\t\tshort: g\n"
		<< "\t\tgets record with given key\n"
		<< "\tgetall\n"
		<< "\t\tshort: ga\n"
		<< "\t\tprints all records in order\n"
		<< "\tdelete key\n"
		<< "\t\tshort: d\n"
		<< "\t\tdelete record with given key\n"
		<< "\tprint\n"
		<< "\t\tshort: p\n"
		<< "\t\tprints DB\n"
		<< "\treorganize\n"
		<< "\t\tshort: re\n"
		<< "\t\treorganizes DB\n"
		<< "\trandom\n"
		<< "\t\tshort: r\n"
		<< "\t\tinserts record with random data\n"
		<< "\tnew name\n"
		<< "\t\tshort: n\n"
		<< "\t\tcreates new db with given name\n"
		<< "\topen name\n"
		<< "\t\tshort: o\n"
		<< "\t\topens existing db with given name\n"
		<< "\tinfo\n"
		<< "\t\tprints info about db\n"
		<< "\texit\n"
		<< "\t\texits program\n"
		<< "\thelp\n" 
		<< "\t\tshort: h\n"
		<< "\t\tprints help"
		<< std::endl;
}

int main()
{
	//INIT
	std::string command, name;
	int IOCounter = 0;
	Record record;
	DataBase db;
	double m, v;
	int key;

	srand(std::time(nullptr));

	//MAIN LOOP
	while (true)
	{
		if (db.file.fileName != "")
			std::cout << "$" << db.file.fileName << " ";
		std::cout << ">> ";
		std::cin >> command;

		if (command == "insert" || command == "i")
		{
			std::cin >> key;
			std::cin >> v;
			std::cin >> m;

			record = { key, v, m };
			std::cout << record << std::endl;
			db.Insert(record);
		}
		else if (command == "info")
			db.Info();
		else if (command == "get" || command == "g")
		{
			std::cin >> key;
			record = db.Get(key);

			if (record.key == key)
				std::cout << "\n" << record << std::endl;
			else
				std::cout << "\nNo record with given key" << std::endl;
		}
		else if (command == "getall" || command == "ga")
		{
			std::cout << "\n";
			do {
				record = db.GetNext();
				if (record.isInitialized() && record.key > 0)
					std::cout << record << std::endl;
			} while (record.isInitialized());
		}
		else if (command == "delete" || command == "d")
		{
			std::cin >> key;

			if (db.Delete(key))
				std::cout << "\nSuccess" << std::endl;
			else
				std::cout << "\nNo record with given key" << std::endl;
		}
		else if (command == "print" || command == "p")
			db.Print();
		else if (command == "reorganize" || command == "re")
			db.Reorganize();
		else if (command == "random" || command == "r")
		{
			std::cout << "\n";
			key = rand() % 1000 + 1;
			m = (rand() % 10000 + 1) / 100.0;
			v = (rand() % 10000 + 1) / 100.0;
			record = { key, v, m };
			std::cout << record << std::endl;
			db.Insert(record);
		}
		else if (command == "new" || command == "n")
		{
			std::cin >> name;
			db.Open(name + "_index", name + "_db", name + "_db_meta", true, &IOCounter);
		}
		else if (command == "open" || command == "o")
		{
			std::cin >> name;
			db.Open(name + "_index", name + "_db", name + "_db_meta", false, &IOCounter);
		}
		else if (command == "exit")
			break;
		else if (command == "help" || command == "h")
			help();
		else
		{
			std::cout << "\nUNKNOWN COMMAND " << command << std::endl;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}

		std::cout << "\nIOCounter: " << IOCounter << std::endl;
		IOCounter = 0;
		std::cout << std::endl;
	}

	db.GenerateMetadata(db.file.fileName + "_meta");
	return 0;
}