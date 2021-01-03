#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <math.h>
#include <fstream>
#include <iostream>
#include "BufferedIntReader.h"
#include "BufferedIntWriter.h"

bool externalSort(std::string inputFilePath, std::string outputFilePath, std::string tempFileDirectory, int availableRAM);