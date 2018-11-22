#pragma once
#include <string>
#include <fstream>
#include <exception>

#include "page.h"

class File
{
	friend void Merge(File& tape1, File& tape2, File& result);
	friend void MergeDummies(File& tape1, File& tape2, File& result);
	friend std::ostream& operator<<(std::ostream& os, File& tape);

public:
	File();
	File(const std::string& fileName, int mode = DEFAULT_INPUT_MODE, unsigned int* counter = nullptr);
	~File();

	void Open(const std::string& fileName, int mode = DEFAULT_INPUT_MODE, unsigned int* counter = nullptr);
	void Close();
	bool eof;

	void WriteNextRecord(const Record& record, bool benchamark = true);
	Record ReadRecord(bool benchamark = true);
	Record ReadNextRecord(bool benchamark = true);

	void SwitchToReadMode();
	void SwitchToWriteMode();

	void IncrementOffset();

	unsigned int dummies;
	unsigned int series;

	static const auto DEFAULT_INPUT_MODE = std::ios::in | std::ios::binary;
	static const auto DEFAULT_OUTPUT_MODE = std::ios::out | std::ios::binary | std::ios::trunc;

	std::string fileName;

protected:
	std::fstream file;
	Page buffer;
	Record last;
	unsigned int* counter;

	unsigned int currentPageId, lastPageId;
	unsigned int pageOffset;

	void ResetPosition();
	void ClearBuffer();
	void ForceWrite(bool benchamark = true);
	bool ReadPage(bool benchamark = true);
	void WritePage(bool benchamark = true);
};

void Merge(File& tape1, File& tape2, File& result);
void MergeDummies(File& tape1, File& tape2, File& result);
std::ostream& operator<<(std::ostream& os, File& tape);