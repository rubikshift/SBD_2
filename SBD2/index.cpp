#include "index.h"

Index::Index(std::string fileName, bool clear) : file()
{
	auto mode = clear ? File::DEFAULT_OUTPUT_MODE : File::DEFAULT_INPUT_MODE;
	file.Open(fileName, mode);

	if (!clear) // wczytaj do bufora
	{

	}
}

int Index::GetPageId(int key)
{
	return -1;
}
