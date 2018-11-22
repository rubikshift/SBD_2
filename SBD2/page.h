#pragma once
#include <memory>
#include <iostream>
#include "record.h"

struct Page
{
	int id;
	bool changed;
	static const unsigned int PAGE_SIZE = 64;							//Page size in records
	static constexpr auto BYTE_SIZE = PAGE_SIZE * Record::RECORD_SIZE;	//Page size in bytes
	char data[BYTE_SIZE];
};