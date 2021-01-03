#pragma once
#include <string>
#include "BufferedIntReader.h"
#include "BufferedIntWriter.h"

bool binaryToText(std::string inputFilePath, std::string outputFilePath, int availableRAM);
bool textToBinary(std::string inputFilePath, std::string outputFilePath, int availableRAM);