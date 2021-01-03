#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "../ExternalMemoryAlgorithms/BTreeMap.h"

const std::string DIRECTORY = "C:/External Memory/Server/";
const int PARAMETER = 160;
const std::string INET_ADDRESS = "127.0.0.1";
const int PORT = 444;
const int MAX_LENGTH = 520;
#include <sdkddkver.h>
#include <iostream>
#include <time.h>
#include <conio.h>
#include <set>
#include <map>
#include <WinSock2.h>
#include <Windows.h>

#define SCK_VERSION2 0x0202
int main()
{
	BTreeMap<std::string, std::string> database(PARAMETER, 0, DIRECTORY);
	long SUCCESSFUL;
	WSAData WindSockData;
	WORD DLLVERSION;

	DLLVERSION = MAKEWORD(2, 1);

	SUCCESSFUL = WSAStartup(DLLVERSION, &WindSockData);
	if(SUCCESSFUL == -1)
		std::cout << "CONNECTION PROBLEMS\n";
	SOCKADDR_IN ADDRESS;
	int AddressSize = sizeof(ADDRESS);

	SOCKET sock_LISTEN;
	SOCKET sock_CONNECTION;

	sock_CONNECTION = socket(AF_INET, SOCK_STREAM, NULL);
	ADDRESS.sin_addr.s_addr = inet_addr(INET_ADDRESS.c_str());
	ADDRESS.sin_family = AF_INET;
	ADDRESS.sin_port = htons(PORT);

	sock_LISTEN = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sock_LISTEN, (SOCKADDR*)&ADDRESS, sizeof(ADDRESS));
	listen(sock_LISTEN, SOMAXCONN);
	while (true)
	{
		std::cout << "WAITING FOR A CONNECTION\n";
		if (sock_CONNECTION = accept(sock_LISTEN, (SOCKADDR*)&ADDRESS, &AddressSize))
		{
			std::cout << "CONNECTION FOUND\n";
			char MESSAGE[MAX_LENGTH];
			char* NEXT;
			while (recv(sock_CONNECTION, MESSAGE, sizeof(MESSAGE), NULL) > 0)
			{
				std::cout << MESSAGE << "\n";
				std::string type(strtok_s(MESSAGE, " ", &NEXT));
				if (type == "I")
				{
					std::string key(strtok_s(NULL, " ", &NEXT));
					std::string value(strtok_s(NULL, " ", &NEXT));
					database.insert({ key, value });
					SUCCESSFUL = send(sock_CONNECTION, "OK", 3, NULL);
				}
				else if (type == "R")
				{
					std::string key(strtok_s(NULL, " ", &NEXT));
					database.erase(key);
					SUCCESSFUL = send(sock_CONNECTION, "OK", 3, NULL);
				}
				else if (type == "F")
				{
					std::string key(strtok_s(NULL, " ", &NEXT));
					std::string value = database.find(key);
					SUCCESSFUL = send(sock_CONNECTION, value.c_str(), value.size() + 1, NULL);
				}
				else if(type == "U")
				{
					std::string key(strtok_s(NULL, " ", &NEXT));
					std::string value(strtok_s(NULL, " ", &NEXT));
					if(database.update({ key, value }))
						SUCCESSFUL = send(sock_CONNECTION, "OK", 3, NULL);
					else
						SUCCESSFUL = send(sock_CONNECTION, "NF", 3, NULL);
				}
				if (SUCCESSFUL == -1)
					std::cout << "CONNECTION PROBLEMS\n";
			}
		}
	}
	WSACleanup();
	return 0;
}

