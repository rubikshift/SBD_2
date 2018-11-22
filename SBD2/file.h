#pragma once
#include <string>
#include <fstream>
#include <exception>

#include "page.h"

class File
{
	public:
		File();
		File(const std::string& fileName, int mode = DEFAULT_INPUT_MODE);
		~File();

		void Open(const std::string& fileName, int mode = DEFAULT_INPUT_MODE);
		void Close();
		bool eof;
	
		void SwitchToReadMode();
		void SwitchToWriteMode();

		Page ReadPage(int pageId);
		void WritePage(Page& page);
	
		unsigned int dummies;
		unsigned int series;
	
		static const auto DEFAULT_INPUT_MODE = std::ios::in | std::ios::binary;
		static const auto DEFAULT_OUTPUT_MODE = std::ios::out | std::ios::binary | std::ios::trunc;
	
		std::string fileName;

	protected:
		std::fstream file;
};