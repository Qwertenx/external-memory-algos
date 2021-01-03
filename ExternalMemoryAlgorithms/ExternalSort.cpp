#include "ExternalSort.h"

const int SYSTEM_BLOCK_SIZE = 1024;
const int MAX_BUFFER_SIZE = 4096;
const int ADDITIONAL_RAM_PER_FILE = 4 * sizeof(int) + sizeof(FILE*);
const int INDEPENDENT_RAM_USAGE = 200;
const int MINIMAL_FILE_NEED = 3;

std::string getTempFileNameSort(const std::string& tempFileDirectory, int index)
{
	return (tempFileDirectory + "/temp" + std::to_string(index) + ".bin");
}

int splitAndSort(FILE* input, const std::string& tempFileDirectory, int availableRAM)
{
	int length = availableRAM / sizeof(int);
	int* buffer = new int[length];
	if (length == 0)
	{
		return 0;
	}
	int counter = 0;
	length = fread(buffer, sizeof(int), length, input);
	while (length > 0)
	{
		std::sort(buffer, buffer + length);
		FILE* current;
		fopen_s(&current, getTempFileNameSort(tempFileDirectory, counter).c_str(), "wb");
		if (current == NULL)
		{
			return 0;
		}
		fwrite(buffer, sizeof(int), length, current);
		fclose(current);
		counter++;
		length = fread(buffer, sizeof(int), length, input);
	}
	fclose(input);
	delete[] buffer;
	return counter;
}

bool mergeFiles(FILE *mainOutput, const std::string& tempFileDirectory, int availableRAM, int numberOfFiles)
{
	int bufferSize = SYSTEM_BLOCK_SIZE;
	int wayOfMerging = std::min(int(availableRAM / (bufferSize * sizeof(int) + ADDITIONAL_RAM_PER_FILE) - 1), numberOfFiles);
	for (int coefficient = 2; coefficient <= availableRAM / SYSTEM_BLOCK_SIZE; coefficient++)
	{
		bufferSize = coefficient * SYSTEM_BLOCK_SIZE;
		wayOfMerging = std::min(int(availableRAM / (bufferSize * sizeof(int) + ADDITIONAL_RAM_PER_FILE) - 1), numberOfFiles);
		if (wayOfMerging < numberOfFiles)
		{
			bufferSize = (coefficient - 1) * SYSTEM_BLOCK_SIZE;
			wayOfMerging = std::min(int(availableRAM / (bufferSize * sizeof(int) + ADDITIONAL_RAM_PER_FILE) - 1), numberOfFiles);
			break;
		}
	}
	if (wayOfMerging < MINIMAL_FILE_NEED - 1)
	{
		return false;
	}
	bool lastMerge = false;
	int firstFileIndex = 0;
	std::priority_queue< std::pair<int, int> > heap;
	BufferedIntReader** readers = new BufferedIntReader*[wayOfMerging];
	for (int i = 0; i < wayOfMerging; i++)
	{
		readers[i] = new BufferedIntReader(nullptr, bufferSize);
	}
	BufferedIntWriter* writer = new BufferedIntWriter(nullptr, bufferSize);
	while (numberOfFiles > 1)
	{
		if (wayOfMerging >= numberOfFiles)
		{
			lastMerge = true;
		}
		for (int i = firstFileIndex; i < firstFileIndex + numberOfFiles; i += wayOfMerging)
		{
			int numberOfInputFiles = 0;
			if (lastMerge)
				writer->setOutput(mainOutput);
			else
			{
				FILE* currentOutput;
				fopen_s(&currentOutput, getTempFileNameSort(tempFileDirectory, (firstFileIndex + numberOfFiles + i / wayOfMerging)).c_str(), "wb");
				if (!currentOutput)
				{
					for (int i = 0; i < wayOfMerging; i++)
						delete readers[i];
					delete[] readers;
					delete writer;
					return false;
				}
				writer->setOutput(currentOutput);
			}
			for (int j = i; j < std::min(i + wayOfMerging, firstFileIndex + numberOfFiles); j++)
			{
				int index = j - i;
				FILE* currentInput;
				fopen_s(&currentInput, getTempFileNameSort(tempFileDirectory, j).c_str(), "rb");
				numberOfInputFiles++;
				if (!currentInput)
				{
					for (int i = 0; i < wayOfMerging; i++)
						delete readers[i];
					delete[] readers;
					delete writer;
					return false;
				}
				readers[index]->setInput(currentInput);
			}
			heap = std::priority_queue< std::pair<int, int> >();
			for (int j = 0; j < numberOfInputFiles; j++)
			{
				int current = readers[j]->next();
				if(current != INT_MAX)
					heap.push({ -current, j });
			}
			int cnt = 0;
			while (!heap.empty())
			{
				writer->writeNext(-heap.top().first);
				int index = heap.top().second;
				heap.pop();
				int current = readers[index]->next();
				if (current == 1)
					cnt++;
				if (current != INT_MAX)
					heap.push({ -current, index });
			}
			for (int j = 0; j < numberOfInputFiles; j++)
			{
				remove(getTempFileNameSort(tempFileDirectory, i + j).c_str());
			}
		}
		firstFileIndex += numberOfFiles;
		numberOfFiles = (numberOfFiles + wayOfMerging - 1) / wayOfMerging;
	}
	for (int i = 0; i < wayOfMerging; i++)
		delete readers[i];
	delete writer;
	delete[] readers;
	return true;
}

bool externalSort(std::string inputFilePath, std::string outputFilePath, std::string tempFileDirectory, int availableRAM)
{
	availableRAM -= INDEPENDENT_RAM_USAGE;
	FILE* input;
	FILE* output;
	fopen_s(&input, inputFilePath.c_str(), "rb");
	if (!input)
	{
		return false;
	}
	int numberOfFiles = splitAndSort(input, tempFileDirectory, availableRAM);
	if (numberOfFiles == 0)
		return false;
	if (numberOfFiles == 1)
	{
		remove(outputFilePath.c_str());
		int result = rename(getTempFileNameSort(tempFileDirectory, 0).c_str(), outputFilePath.c_str());
		if (result == 0)
			return true;
		else
			return false;
	}
	fopen_s(&output, outputFilePath.c_str(), "wb");
	if (!output)
	{
		return false;
	}
	return mergeFiles(output, tempFileDirectory, availableRAM, numberOfFiles);
}