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
		void ClearBuffer();
		void CreateSpace(int pages);
	
		void SwitchToReadMode();
		void SwitchToWriteMode();

		bool ReadToBuffer(int pageId);
		void WriteBuffer();

	
		static const auto DEFAULT_INPUT_MODE = std::ios::in | std::ios::binary;
		static const auto DEFAULT_OUTPUT_MODE = std::ios::out | std::ios::binary | std::ios::trunc;
		static const auto DEFAULT_INPUT_OUTPUT_MODE = std::ios::out | std::ios::in | std::ios::binary;
	
		std::string fileName;
		Page buffer;

	protected:
		std::fstream file;
};