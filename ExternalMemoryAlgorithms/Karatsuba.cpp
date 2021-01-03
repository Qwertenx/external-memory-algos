#include "Karatsuba.h"
const int NUMBER_OF_MULTIPLIERS = 2;
const int NUMBER_OF_DISTINCT_ADDENDS = 3;
const int DEFAULT_BLOCK_SIZE = 1024;
const int QUADRATIC_BOUND = 30;
std::string getTempFileNameKaratsuba(const std::string& tempFileDirectory, int index)
{
	return (tempFileDirectory + "/temp" + std::to_string(index) + ".bin");
}

int normalize(int x)
{
	if (x == INT_MAX)
		return 0;
	else
		return x;
}
void shift(std::vector<int>& polynom, int number)
{
	std::vector<int> answer;
	answer.reserve(polynom.size() + number);
	while (number--)
		answer.push_back(0);
	for (int i = 0; i < polynom.size(); i++)
		answer.push_back(polynom[i]);
	answer.swap(polynom);
}
void add(std::vector<int>& firstPolynom, const std::vector<int>& secondPolynom)
{
	if (secondPolynom.size() > firstPolynom.size())
		firstPolynom.resize(secondPolynom.size(), 0);
	for (int i = 0; i < secondPolynom.size(); i++)
		firstPolynom[i] += secondPolynom[i];
}
void subtract(std::vector<int>& firstPolynom, const std::vector<int>& secondPolynom)
{
	if (secondPolynom.size() > firstPolynom.size())
		firstPolynom.resize(secondPolynom.size(), 0);
	for (int i = 0; i < secondPolynom.size(); i++)
		firstPolynom[i] -= secondPolynom[i];
}
void multiply(std::vector<int>& firstPolynom, std::vector<int>& secondPolynom)
{
	if (firstPolynom.size() < secondPolynom.size())
		swap(firstPolynom, secondPolynom);
	if (secondPolynom.size() == 0)
	{
		firstPolynom.resize(0);
		return;
	}
	if (secondPolynom.size() == 1)
	{
		for (int i = 0; i < firstPolynom.size(); i++)
			firstPolynom[i] *= secondPolynom[0];
		return;
	}
	if (firstPolynom.size() <= QUADRATIC_BOUND)
	{
		std::vector<int> result(firstPolynom.size() + secondPolynom.size() - 1, 0);
		for (int i = 0; i < firstPolynom.size(); i++)
			for (int j = 0; j < secondPolynom.size(); j++)
				result[i + j] += firstPolynom[i] * secondPolynom[j];
		result.swap(firstPolynom);
		return;
	}
	int half = firstPolynom.size() / 2;
	std::vector<int> firstPolynomSecondPart;
	firstPolynomSecondPart.reserve(firstPolynom.size() - half);
	std::vector<int> secondPolynomSecondPart;
	secondPolynomSecondPart.reserve(std::max(0, (int)secondPolynom.size() - half));
	for (int i = half; i < firstPolynom.size(); i++)
		firstPolynomSecondPart.push_back(firstPolynom[i]);
	for (int i = half; i < secondPolynom.size(); i++)
		secondPolynomSecondPart.push_back(secondPolynom[i]);
	firstPolynom.resize(half);
	secondPolynom.resize(half);
	std::vector<int> firstPolynomCopy = firstPolynom;
	std::vector<int> secondPolynomCopy = secondPolynom;
	multiply(firstPolynom, secondPolynom);
	add(firstPolynomCopy, firstPolynomSecondPart);
	add(secondPolynomCopy, secondPolynomSecondPart);
	multiply(firstPolynomSecondPart, secondPolynomSecondPart);
	multiply(firstPolynomCopy, secondPolynomCopy);
	subtract(firstPolynomCopy, firstPolynom);
	subtract(firstPolynomCopy, firstPolynomSecondPart);
	shift(firstPolynomCopy, half);
	shift(firstPolynomSecondPart, 2 * half);
	add(firstPolynom, firstPolynomCopy);
	add(firstPolynom, firstPolynomSecondPart);
	secondPolynom.reserve(secondPolynom.size() + secondPolynomSecondPart.size());
	for (int i = 0; i < secondPolynomSecondPart.size(); i++)
		secondPolynom.push_back(secondPolynomSecondPart[i]);
}

bool multiply(const std::string& firstInputFilePath, const std::string& secondInputFilePath, const std::string& outputFilePath, const std::string& tempFileDirectory, int bound, int& tempFileIndex)
{
	FILE* firstInput;
	FILE* firstCopy;
	FILE* secondCopy;
	FILE* secondInput;
	FILE* output;
	if (fopen_s(&firstInput, firstInputFilePath.c_str(), "rb"))
		return false;
	if (fopen_s(&secondInput, secondInputFilePath.c_str(), "rb"))
		return false;
	if (fopen_s(&firstCopy, firstInputFilePath.c_str(), "rb"))
		return false;
	if (fopen_s(&secondCopy, secondInputFilePath.c_str(), "rb"))
		return false;
	int firstLength, secondLength;
	std::unique_ptr<BufferedIntReader> firstReader(new BufferedIntReader(firstInput, DEFAULT_BLOCK_SIZE));
	std::unique_ptr<BufferedIntReader> secondReader(new BufferedIntReader(secondInput, DEFAULT_BLOCK_SIZE));
	std::unique_ptr<BufferedIntReader> firstReaderCopy(new BufferedIntReader(firstCopy, DEFAULT_BLOCK_SIZE));
	std::unique_ptr<BufferedIntReader> secondReaderCopy(new BufferedIntReader(secondCopy, DEFAULT_BLOCK_SIZE));
	firstLength = firstReader->next();
	secondLength = secondReader->next();
	firstReaderCopy->next();
	secondReaderCopy->next();
	if (firstLength < secondLength)
	{
		std::swap(firstInput, secondInput);
		std::swap(firstLength, secondLength);
		firstReader.swap(secondReader);
		firstReaderCopy.swap(secondReaderCopy);
	}
	if (secondLength == 0)
	{
		if (fopen_s(&output, outputFilePath.c_str(), "wb"))
			return false;
		int length = 0;
		fwrite(&length, sizeof(int), 1, output);
		fclose(output);
		return true;
	}
	if (secondLength == 1)
	{
		if (fopen_s(&output, outputFilePath.c_str(), "wb"))
			return false;
		std::unique_ptr<BufferedIntWriter> mainOutput(new BufferedIntWriter(output, DEFAULT_BLOCK_SIZE));
		int length = 1;
		mainOutput->writeNext(length);
		int scalar = secondReader->next();
		for (int i = 0; i < firstLength; i++)
		{
			int number = firstReader->next();
			number *= scalar;
			mainOutput->writeNext(number);
		}
		return true;
	}
	if (firstLength <= bound)
	{
		if (fopen_s(&output, outputFilePath.c_str(), "wb"))
			return false;
		std::vector<int> firstPolynom, secondPolynom;
		firstPolynom.reserve(firstLength);
		secondPolynom.reserve(secondLength);
		for (int i = 0; i < firstLength; i++)
			firstPolynom.push_back(firstReader->next());
		for (int i = 0; i < secondLength; i++)
			secondPolynom.push_back(secondReader->next());
		multiply(firstPolynom, secondPolynom);
		std::unique_ptr<BufferedIntWriter> mainOutput(new BufferedIntWriter(output, DEFAULT_BLOCK_SIZE));
		mainOutput->writeNext(firstPolynom.size());
		for (int i = 0; i < firstPolynom.size(); i++)
			mainOutput->writeNext(firstPolynom[i]);
		return true;
	}
	int half = firstLength / 2;
	std::string subInputsNames[NUMBER_OF_DISTINCT_ADDENDS][NUMBER_OF_MULTIPLIERS];
	std::string subOutputsNames[NUMBER_OF_DISTINCT_ADDENDS];
	std::unique_ptr<BufferedIntWriter> subInputs[NUMBER_OF_DISTINCT_ADDENDS][NUMBER_OF_MULTIPLIERS];
	std::unique_ptr<BufferedIntReader> subOutputs[NUMBER_OF_DISTINCT_ADDENDS];
	for (int i = 0; i < NUMBER_OF_DISTINCT_ADDENDS; i++)
	{
		for (int j = 0; j < NUMBER_OF_MULTIPLIERS; j++)
		{
			FILE* current;
			if (fopen_s(&current, (subInputsNames[i][j] = getTempFileNameKaratsuba(tempFileDirectory, tempFileIndex)).c_str(), "wb"))
				return false;
			tempFileIndex++;
			subInputs[i][j] = std::unique_ptr<BufferedIntWriter>(new BufferedIntWriter(current, DEFAULT_BLOCK_SIZE));
			if (i == 0)
			{
				if(j == 0)
					subInputs[i][j]->writeNext(half);
				else if(j == 1)
					subInputs[i][j]->writeNext(std::min(half, secondLength));
			}
			else
			{
				if (j == 0)
					subInputs[i][j]->writeNext(firstLength - half);
				else if (j == 1)
				{
					if (i == 1)
						subInputs[i][j]->writeNext(std::max(std::min(half, secondLength), secondLength - half));
					else
						subInputs[i][j]->writeNext(std::max(0, secondLength - half));
				}
			}
		}
	}
	for (int i = 0; i < firstLength; i++)
	{
		bool needToWriteSecond = false;
		int firstNumber = normalize(firstReader->next());
		int temp = secondReader->next();
		if (temp != INT_MAX)
			needToWriteSecond = true;
		int secondNumber = normalize(temp);
		if (i < half)
		{
			subInputs[0][0]->writeNext(firstNumber);
			if(needToWriteSecond)
				subInputs[0][1]->writeNext(secondNumber);
		}
		else
		{
			subInputs[2][0]->writeNext(firstNumber);
			if (needToWriteSecond)
				subInputs[2][1]->writeNext(secondNumber);
			int thirdNumber, fourthNumber;
			if (i == firstLength - 1 && firstLength % 2 == 1)
			{
				thirdNumber = 0;
				fourthNumber = 0;
			}
			else
			{
				thirdNumber = normalize(firstReaderCopy->next());
				int temp = secondReaderCopy->next();
				if (temp != INT_MAX)
					needToWriteSecond = true;
				fourthNumber = normalize(temp);
			}
			subInputs[1][0]->writeNext(firstNumber + thirdNumber);
			if (needToWriteSecond)
				subInputs[1][1]->writeNext(secondNumber + fourthNumber);
		}
	}
	for (int i = 0; i < NUMBER_OF_DISTINCT_ADDENDS; i++)
	{
		for (int j = 0; j < NUMBER_OF_MULTIPLIERS; j++)
		{
			subInputs[i][j].reset();
		}
	}
	firstReader.reset();
	firstReaderCopy.reset();
	secondReader.reset();
	secondReaderCopy.reset();
	std::unique_ptr<BufferedIntReader> zeroOutputCopy = nullptr, secondOutputCopy = nullptr;
	for (int i = 0; i < NUMBER_OF_DISTINCT_ADDENDS; i++)
	{
		subOutputsNames[i] = getTempFileNameKaratsuba(tempFileDirectory, tempFileIndex);
		tempFileIndex++;
		if (!multiply(subInputsNames[i][0], subInputsNames[i][1], subOutputsNames[i], tempFileDirectory, bound, tempFileIndex))
			return false;
	}
	for (int i = 0; i < NUMBER_OF_DISTINCT_ADDENDS; i++)
	{
		FILE *current = nullptr, *currentCopy = nullptr;
		if (fopen_s(&current, subOutputsNames[i].c_str(), "rb"))
		{
			return false;
		}
		if ((i == 0 || i == 2) && fopen_s(&currentCopy, subOutputsNames[i].c_str(), "rb"))
		{
			return false;
		}
		subOutputs[i] = std::unique_ptr<BufferedIntReader>(new BufferedIntReader(current, DEFAULT_BLOCK_SIZE));
		subOutputs[i]->next();
		if (i == 0)
			zeroOutputCopy = std::unique_ptr<BufferedIntReader>(new BufferedIntReader(currentCopy, DEFAULT_BLOCK_SIZE));
		else if (i == 2)
			secondOutputCopy = std::unique_ptr<BufferedIntReader>(new BufferedIntReader(currentCopy, DEFAULT_BLOCK_SIZE));
	}
	zeroOutputCopy->next();
	secondOutputCopy->next();
	if (fopen_s(&output, outputFilePath.c_str(), "wb"))
		return false;
	std::unique_ptr<BufferedIntWriter> mainOutput(new BufferedIntWriter(output, DEFAULT_BLOCK_SIZE));
	mainOutput->writeNext(firstLength + secondLength - 1);
	for (int i = 0; i < firstLength + secondLength - 1; i++)
	{
		int result = 0;
		result += normalize(subOutputs[0]->next());
		if (i >= half)
		{
			result += normalize(subOutputs[1]->next());
			result -= normalize(zeroOutputCopy->next());
			result -= normalize(secondOutputCopy->next());
		}
		if (i >= 2 * half)
		{
			result += normalize(subOutputs[2]->next());
		}
		mainOutput->writeNext(result);
	}
	return true;
}

bool multiply(const std::string firstInputFilePath, const std::string secondInputFilePath, const std::string outputFilePath, const std::string tempFileDirectory, const int bound)
{
	int counter = 0;
	return multiply(firstInputFilePath, secondInputFilePath, outputFilePath, tempFileDirectory, bound, counter);
}