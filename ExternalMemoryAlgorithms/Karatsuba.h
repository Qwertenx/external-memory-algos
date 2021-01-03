#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include "time.h"
#include "BufferedIntReader.h"
#include "BufferedIntWriter.h"

void multiply(std::vector<int>& firstPolynom, std::vector<int>& secondPolynom);
bool multiply(const std::string& firstInputFilePath, const std::string& secondInputFilePath, const std::string& outputFilePath, const std::string& tempFileDirectory, const int bound, int &tempFileIndex);
bool multiply(const std::string firstInputFilePath, const std::string secondInputFilePath, const std::string outputFilePath, const std::string tempFileDirectory, const int bound);