#include <iostream>
#include "record.h"
#include "index.h"

void help()
{
	std::cout << "HELP:\n"
		<< "\tinsert key m v\n"
		<< "\t\t if key exists updates record, otherwise adds record\n"
		<< "\tdelete key\n"
		<< "\t\tdelete record with given key\n"
		<< "\tprint\n"
		<< "\t\tprints DB\n"
		<< "\treorganize\n"
		<< "\t\treorganizes DB\n"
		<< "\trandom\n"
		<< "\t\tInserts record with random data\n"
		<< "\texit\n"
		<< "\t\texits program\n"
		<< "\thelp\n" 
		<< "\t\tprints help" 
		<< std::endl;
}

int main()
{
	std::string command;

	while (true)
	{
		std::cout << "> ";
		std::cin >> command;

		if (command == "insert")
		{

		}
		else if (command == "delete")
		{

		}
		else if (command == "print")
			;//db.Print();
		else if (command == "reorganize")
			;//db.Reogranize();
		else if (command == "random")
		{

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