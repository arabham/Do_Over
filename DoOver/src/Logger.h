#pragma once
#include <string>

class Logger
{
public:
	static void Log(const std::string& message);
	static void Err(const std::string& message);
};

