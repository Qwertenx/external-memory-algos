#include "BufferedIntReader.h"

BufferedIntReader::BufferedIntReader(FILE* input, int length)
{
	this->input = input;
	this->length = length;
	lastPointer = length;
	defaultLength = length;
	buffer = new int[length];
}
BufferedIntReader::~BufferedIntReader()
{
	fclose(input);
	delete buffer;
}
int BufferedIntReader::next()
{
	if (lastPointer == length)
	{
		length = fread(buffer, sizeof(int), length, input);
		lastPointer = 0;
	}
	if (length == 0)
	{
		return INT_MAX;
	}
	return buffer[lastPointer++];
}
void BufferedIntReader::setInput(FILE* newInput)
{
	if(input != nullptr)
		fclose(input);
	input = newInput;
	lastPointer = defaultLength;
	length = defaultLength;
}