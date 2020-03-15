#pragma once
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include "LoggerAPI.h"

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

class Logger : public LoggerAPI
{
public:
	friend LoggerAPI;

	void logInfo(const std::string &log) override;
	void logInfo(const char *log) override;

	void logCritical(const std::string &log) override;

	void logError(const std::string &log) override;
	void logError(const char *log) override;

	void logWarning(const std::string &log) override;

	~Logger() = default;

private:
	Logger();

	std::shared_ptr<spdlog::logger> m_logger;
};



