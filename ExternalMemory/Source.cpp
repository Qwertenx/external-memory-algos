#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../ExternalMemoryAlgorithms/ExternalSort.h"
#include "../ExternalMemoryAlgorithms/Conversion.h"
#include "../ExternalMemoryAlgorithms/BTreeSet.h"
#include "../ExternalMemoryAlgorithms/BTreeMap.h"
#include "../ExternalMemoryAlgorithms/Karatsuba.h"

#include <time.h>
#include <conio.h>
#include <iostream>
#include <set>
#include <map>
#include <sdkddkver.h>
#include <WinSock2.h>
#include <Windows.h>

#define SCK_VERSION2 0x0202
const std::string TEST_DIRECTORY = "C:/External Memory/Test/BTree/";
const std::string INET_ADDRESS = "127.0.0.1";
const int MAX_LENGTH = 255;
const int PORT = 444;

void performRequest(const SOCKET& sock, std::string request, char*& response)
{
	response[0] = '\0';
	long SUCCESSFUL;
	SUCCESSFUL = send(sock, request.c_str(), request.size() + 1, NULL);
	if (SUCCESSFUL == -1)
	{
		std::cout << "Connection problems\n";
		return;
	}
	SUCCESSFUL = recv(sock, response, MAX_LENGTH + 1, NULL);
	if (SUCCESSFUL == -1)
	{
		std::cout << "Connection problems\n";
		return;
	}
}
void manageDatabase()
{
	long SUCCESSFUL;
	WSAData WinSockData;
	WORD DLLVersion;
	DLLVersion = MAKEWORD(2, 1);
	SUCCESSFUL = WSAStartup(DLLVersion, &WinSockData);
	if (SUCCESSFUL == -1)
	{
		std::cout << "Connection problems\n";
		return;
	}
	char* response = new char[MAX_LENGTH + 1];

	SOCKADDR_IN ADDRESS;

	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, NULL);

	ADDRESS.sin_addr.s_addr = inet_addr(INET_ADDRESS.c_str());
	ADDRESS.sin_family = AF_INET;
	ADDRESS.sin_port = htons(PORT);

	connect(sock, (SOCKADDR*)&ADDRESS, sizeof(ADDRESS));

	while (true)
	{
		bool didInput = false;
		std::string option;
		std::cout << "Select one of the following options: \n";
		std::cout << "1. Insert\n";
		std::cout << "2. Remove\n";
		std::cout << "3. Find\n";
		std::cout << "4. Update\n";
		std::cout << "5. Back\n";
		getline(std::cin, option);
		if (option.length() != 1)
		{
			std::cout << "Incorrect option\n";
		}
		else
		{
			std::string key, value, request;
			switch (option[0])
			{
			case '1':
				std::cout << "Enter space separated string's pair, key and value: ";
				std::cin >> key >> value;
				didInput = true;
				if (key.length() > MAX_LENGTH || value.length() > MAX_LENGTH)
				{
					std::cout << "Max length for key and value is " << MAX_LENGTH << "\n";
					return;
				}
				request = "I " + key + " " + value;
				performRequest(sock, request, response);
				std::cout << response << "\n";
				break;
			case '2':
				std::cout << "Enter key: ";
				std::cin >> key;
				didInput = true;
				if (key.length() > MAX_LENGTH)
				{
					std::cout << "Max length for key and value is " << MAX_LENGTH << "\n";
					return;
				}
				request = "R " + key;
				performRequest(sock, request, response);
				std::cout << response << "\n";
				break;
			case '3':
				std::cout << "Enter key: ";
				std::cin >> key;
				didInput = true;
				if (key.length() > MAX_LENGTH)
				{
					std::cout << "Max length for key and value is " << MAX_LENGTH << "\n";
					return;
				}
				request = "F " + key;
				performRequest(sock, request, response);
				if (strcmp(response, ""))
					std::cout << response << "\n";
				else
					std::cout << "Not found\n";
				break;
			case '4':
				std::cout << "Enter space separated string's pair, key and value: ";
				std::cin >> key >> value;
				didInput = true;
				if (key.length() > MAX_LENGTH || value.length() > MAX_LENGTH)
				{
					std::cout << "Max length for key and value is " << MAX_LENGTH << "\n";
					return;
				}
				request = "U " + key + " " + value;
				performRequest(sock, request, response);
				if (strcmp(response, "OK"))
					std::cout << "Not found\n";
				else
					std::cout << response << "\n";
				break;
			case '5':
				WSACleanup();
				return;
			default:
				std::cout << "Incorrect option\n";
				break;
			}
		}
		if(didInput)
			getline(std::cin, option);
	}
	WSACleanup();
}

void sortFile()
{
	std::string input, output, tempDirectory, tempStringForRAM;
	int availableRAM;
	std::cout << "Specify the input file path:\n";
	getline(std::cin, input);
	std::cout << "Specify the output file path:\n";
	getline(std::cin, output);
	std::cout << "Specify the directory for temporary files:\n";
	getline(std::cin, tempDirectory);
	std::cout << "Specify the usabe amount of RAM in bytes:\n";
	getline(std::cin, tempStringForRAM);
	try
	{
		availableRAM = std::stoi(tempStringForRAM);
	}
	catch (std::invalid_argument)
	{
		std::cout << "Invalid string\n";
		return;
	}
	catch (std::out_of_range)
	{
		std::cout << "Too big number\n";
		return;
	}
	if (externalSort(input, output, tempDirectory, availableRAM))
		std::cout << "Succesfully sorted\n";
	else
		std::cout << "Not enough RAM or cannot access given locations\n";
}

void multiplyTwoPolynoms()
{
	std::string firstInput, secondInput, output, tempDirectory, tempBound;
	int bound;
	std::cout << "Specify the first input file path:\n";
	getline(std::cin, firstInput);
	std::cout << "Specify the second input file path:\n";
	getline(std::cin, secondInput);
	std::cout << "Specify the output file path:\n";
	getline(std::cin, output);
	std::cout << "Specify the directory for temporary files:\n";
	getline(std::cin, tempDirectory);
	std::cout << "Specify the size of polynoms, for which to start regular Karatsuba's algorithm:\n";
	getline(std::cin, tempBound);
	try
	{
		bound = std::stoi(tempBound);
	}
	catch (std::invalid_argument)
	{
		std::cout << "Invalid string\n";
		return;
	}
	catch (std::out_of_range)
	{
		std::cout << "Too big number\n";
		return;
	}
	if (multiply(firstInput, secondInput, output, tempDirectory, bound))
		std::cout << "Succesfully multiplied\n";
	else
		std::cout << "Not enough RAM or cannot access given locations\n";
}
int main()
{
	clock_t tStart = clock();
	while (true)
	{
		std::string option;
		std::cout << "Select one of the following options: \n";
		std::cout << "1. Manage database\n";
		std::cout << "2. Sort file\n";
		std::cout << "3. Multiply two polynoms\n";
		std::cout << "4. Exit\n";
		getline(std::cin, option);
		if (option.length() != 1)
		{
			std::cout << "Incorrect option\n";
		}
		else
		{
			switch (option[0])
			{
			case '1':
				manageDatabase();
				break;
			case '2':
				sortFile();
				break;
			case '3':
				multiplyTwoPolynoms();
				break;
			case '4':
				return 0;
			default:
				std::cout << "Incorrect option\n";
				break;
			}
		}
	}
	printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
	_getch();
	return 0;
}