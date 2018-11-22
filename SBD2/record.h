#pragma once
#include <limits>
#include <iostream>

struct Record
{
	Record(int key, double v = UNINIT, double m = UNINIT); // v == UNINIT or m == UNINIT => uninitialized record
	bool isInitialized() const;

	int key, ptr;
	double v, m;

	static constexpr double UNINIT = -1 * std::numeric_limits<double>::infinity();
	static const unsigned int RECORD_SIZE = sizeof(v) + sizeof(m) + sizeof(key) + sizeof(ptr);

};

std::ostream& operator<<(std::ostream& os, const Record& record);