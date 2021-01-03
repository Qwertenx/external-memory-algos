#pragma once
#include <stdio.h>
#include <climits>

class BufferedIntWriter
{
	private:
		FILE* output;
		int lastPointer;
		int length;
		int* buffer;
	public:
		BufferedIntWriter(FILE* output, int length);
		~BufferedIntWriter();
		void writeNext(int number);
		void setOutput(FILE* newOutput);
};