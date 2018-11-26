#include <iostream>
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
		std::cout << ">> ";
		std::cin >> command;

		if (command == "insert")
		{
			std::cin >> key;
			std::cin >> v;
			std::cin >> m;

			record = { key, v, m };

			db.Insert(record);
		}
		else if (command == "get")
		{
			std::cin >> key;
			record = db.Get(key);

			if (record.key == key)
				std::cout << record << std::endl;
			else
				std::cout << "No record with given key" << std::endl;
		}
		else if (command == "delete")
		{
			std::cin >> key;

			if (db.Delete(key))
				std::cout << "Success" << std::endl;
			else
				std::cout << "No record with given key" << std::endl;
		}
		else if (command == "print")
			db.Print();
		else if (command == "reorganize")
			;//db.Reogranize();
		else if (command == "random")
		{

		}
		else if (command == "new")
		{
			std::cin >> name;
			db.Open(name + "_index", name + "_db", true);
		}
		else if (command == "open")
		{
			std::cin >> name;
			db.Open(name + "_index", name + "_db", false);
		}
		else if (command == "exit")
			break;
		else if (command == "help")
			help();
		else
			std::cout << "\nUNKNOWN COMMAND" << std::endl;
		std::cout << std::endl;
	}

	return 0;
}