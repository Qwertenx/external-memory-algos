#include "BufferedIntWriter.h"

BufferedIntWriter::BufferedIntWriter(FILE* output, int length)
{
	this->output = output;
	this->length = length;
	lastPointer = 0;
	buffer = new int[length];
}
BufferedIntWriter::~BufferedIntWriter()
{
	fwrite(buffer, sizeof(int), lastPointer, output);
	fclose(output);
	delete buffer;
}
void BufferedIntWriter::writeNext(int number)
{
	if (lastPointer == length)
	{
		fwrite(buffer, sizeof(int), length, output);
		lastPointer = 0;
	}
	buffer[lastPointer++] = number;
}
void BufferedIntWriter::setOutput(FILE* newOutput)
{
	if (output != nullptr)
	{
		fwrite(buffer, sizeof(int), lastPointer, output);
		fclose(output);
	}
	output = newOutput;
	lastPointer = 0;
}