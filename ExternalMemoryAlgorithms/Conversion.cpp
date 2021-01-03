#include "Conversion.h"

bool binaryToText(std::string inputFilePath, std::string outputFilePath, int availableRAM)
{
	FILE* input;
	FILE* output;
	fopen_s(&input, inputFilePath.c_str(), "rb");
	fopen_s(&output, outputFilePath.c_str(), "w");
	if (!input || !output)
		return false;
	int bufferSize = availableRAM / 4;
	int* buffer = new int[bufferSize];
	int length = 0;
	while (length = fread(buffer, sizeof(int), bufferSize, input))
	{
		for (int i = 0; i < length; i++)
			fprintf(output, "%d ", buffer[i]);
	}
	delete[] buffer;
	fclose(input);
	fclose(output);
	return true;
}
bool textToBinary(std::string inputFilePath, std::string outputFilePath, int availableRAM)
{
	FILE* input;
	FILE* output;
	fopen_s(&input, inputFilePath.c_str(), "r");
	fopen_s(&output, outputFilePath.c_str(), "wb");
	if (!input || !output)
		return false;
	int current;
	while (fscanf_s(input, "%d", &current) == 1)
	{
		fwrite(&current, sizeof(int), 1, output);
	}
	fclose(input);
	fclose(output);
	return true;
}