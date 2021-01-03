#pragma once

#include <stdio.h>
#include <climits>

class BufferedIntReader
{
	private:
		FILE* input;
		int lastPointer;
		int length;
		int defaultLength;
		int* buffer;
	public:
		BufferedIntReader(FILE* input, int length);
		~BufferedIntReader();
		int next();
		void setInput(FILE* newInput);
};