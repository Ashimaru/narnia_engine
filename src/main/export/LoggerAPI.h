#pragma once
#include <memory>
#include <string>

class LoggerAPI;
using LoggerAPIPtr = std::shared_ptr<LoggerAPI>;
class LoggerAPI
{
public:
	static LoggerAPIPtr getLogger();
	static void freeLogger();
	
	virtual void logInfo(const std::string &log) = 0;
	virtual void logInfo(const char *log) = 0;

	virtual void logCritical(const std::string &log) = 0;

	virtual void logError(const std::string &log) = 0;
	virtual void logError(const char *log) = 0;

	virtual void logWarning(const std::string &log) = 0;

	virtual ~LoggerAPI() {};
protected:
	static LoggerAPIPtr m_instance;
};