#include "Logger.h"

namespace {
const std::string loggerName = "Renderer Logger";
const std::string logPath = "./renderLogs.log";
}

LoggerAPIPtr LoggerAPI::m_instance = nullptr;

LoggerAPIPtr LoggerAPI::getLogger()
{
	if (m_instance == nullptr)
	{
		m_instance = LoggerPtr(new Logger());
	}
	return m_instance;
}

void Logger::logInfo(const std::string & log)
{
	m_logger->info(log);
}

void Logger::logInfo(const char * log)
{
	m_logger->info(log);
}

void Logger::logCritical(const std::string & log)
{
	m_logger->critical(log);
}

void Logger::logError(const std::string & log)
{
	m_logger->error(log);
}

void Logger::logError(const char * log)
{
	m_logger->error(log);
}


void Logger::logWarning(const std::string & log)
{
	m_logger->warn(log);
}

Logger::Logger()
{
	remove(logPath.c_str());
	m_logger = spdlog::basic_logger_mt(loggerName, logPath);
	m_logger->flush_on(spdlog::level::info);
}
