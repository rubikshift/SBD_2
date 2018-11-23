#include <iomanip>
#include "record.h"

Record::Record(int key, double v, double m) : key(key), v(v), m(m), ptr(NO_PTR)
{
}

bool Record::isInitialized() const
{
	return v != UNINIT && m != UNINIT;
}

std::ostream & operator<<(std::ostream & os, const Record & record)
{
	//os << "Ek: " << std::setw(10) << std::left << std::setprecision(5) << record.Energy();
	os << "Key: " << std::setw(10) << std::left << record.key;
	os << "v: " << std::setw(10) << std::left << std::setprecision(5) << record.v;
	os << "m: " << std::setw(10) << std::left << std::setprecision(5) << record.m;
	os << "Key: " << std::setw(10) << std::left << record.ptr;
	return os;
}
