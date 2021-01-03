#pragma once
#include "BTreeMap.h"
class Database
{
private:
	BTreeMap<std::string, std::tuple> tables;
public:
	Database();
	~Database();
};

