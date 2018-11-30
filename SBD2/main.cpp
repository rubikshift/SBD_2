#include <iostream>
#include <limits>
#include "database.h"

void help()
{
	std::cout << "HELP:\n"
		<< "\tinsert key v m\n"
		<< "\t\tif key exists updates record, otherwise adds record\n"
		<< "\tget key\n"
		<< "\t\tgets record with gieven key\n"
		<< "\tdelete key\n"
		<< "\t\tdelete record with given key\n"
		<< "\tprint\n"
		<< "\t\tprints DB\n"
		<< "\treorganize\n"
		<< "\t\treorganizes DB\n"
		<< "\trandom\n"
		<< "\t\tinserts record with random data\n"
		<< "\tnew name\n"
		<< "\t\tcreates new db with given name\n"
		<< "\topen name\n"
		<< "\t\topens existing db with given name\n"
		<< "\texit\n"
		<< "\t\texits program\n"
		<< "\thelp\n" 
		<< "\t\tprints help" 
		<< std::endl;
}

int main()
{
	//INIT
	std::string command, name;
	Record record;
	DataBase db;
	double m, v;
	int key;

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

			db.Insert(record);
		}
		else if (command == "get" || command == "g")
		{
			std::cin >> key;
			record = db.Get(key);

			if (record.key == key)
				std::cout << record << std::endl;
			else
				std::cout << "No record with given key" << std::endl;
		}
		else if (command == "delete" || command == "d")
		{
			std::cin >> key;

			if (db.Delete(key))
				std::cout << "Success" << std::endl;
			else
				std::cout << "No record with given key" << std::endl;
		}
		else if (command == "print" || command == "p")
			db.Print();
		else if (command == "reorganize")
			db.Reorganize();
		else if (command == "random")
		{

		}
		else if (command == "new" || command == "n")
		{
			std::cin >> name;
			db.Open(name + "_index", name + "_db", name + "_db_meta", true);
		}
		else if (command == "open" || command == "o")
		{
			std::cin >> name;
			db.Open(name + "_index", name + "_db", name + "_db_meta", false);
		}
		else if (command == "exit")
			break;
		else if (command == "help" || command == "h")
			help();
		else
		{
			std::cout << "\nUNKNOWN COMMAND" << std::endl;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}

		std::cout << std::endl;
	}

	db.GenerateMetadata(db.file.fileName + "_meta");
	return 0;
}