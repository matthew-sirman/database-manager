#include "../../include/database/Logger.h"

std::ostream* Logger::errStream = &std::cerr;
std::ostream* Logger::logStream = &std::cout;
std::ostream* Logger::changelogStream = &std::cout;
std::stringstream* Logger::ss = new std::stringstream();
std::mutex Logger::errStreamLock = std::mutex();
std::mutex Logger::logStreamLock = std::mutex();
std::mutex Logger::changelogStreamLock = std::mutex();
std::mutex Logger::streamLock = std::mutex();

void Logger::logError(const std::string& e, unsigned lineNumber,
                      const std::string& file, bool safe) {
  std::lock_guard<std::mutex> lock(errStreamLock);
  *errStream << timestamp();
  if (file != "") {
    *errStream << file << ":";
  }
  if (lineNumber != -1) {
    *errStream << lineNumber << ": ";
  }
  *errStream << e << std::endl;

  if (!safe) {
    exit(1);
  }
}

void Logger::setErrStream(std::ostream* errStream) {
  Logger::errStream = errStream;
}

void Logger::log(const std::string& e, unsigned lineNumber,
                 const std::string& file) {
  std::lock_guard<std::mutex> lock(logStreamLock);
  *logStream << timestamp();
  if (file != "") {
    *logStream << file << ":";
  }
  if (lineNumber != -1) {
    *logStream << lineNumber << ": ";
  }
  *logStream << e << std::endl;
}

void Logger::setLogStream(std::ostream* logStream) {
  Logger::logStream = logStream;
}

void Logger::changelog(const std::string& e, unsigned lineNumber,
                       const std::string& file) {
  std::lock_guard<std::mutex> lock(changelogStreamLock);
  *changelogStream << timestamp();
  if (file != "") {
    *changelogStream << file << ":";
  }
  if (lineNumber != -1) {
    *changelogStream << lineNumber << ": ";
  }
  *changelogStream << e << std::endl;
}

void Logger::setChangelogStream(std::ostream* changelogStream) {
  Logger::changelogStream = changelogStream;
}

std::string Logger::timestamp() {
  std::stringstream s;
  std::time_t now = time(nullptr);
  std::tm* timePoint = std::localtime(&now);
  s << "[" << std::put_time(timePoint, "%d/%m/%Y %H:%M:%S") << "] ";
  return s.str();
}

std::unique_ptr<std::stringstream, void (*)(std::stringstream*)> Logger::writeStream() {
  streamLock.lock();
  std::unique_ptr<std::stringstream, void (*)(std::stringstream*)> out =
      std::unique_ptr<std::stringstream, void (*)(std::stringstream*)>(
          ss, [](std::stringstream* ptr) {
          streamLock.unlock();
          });
    return out;
}

void Logger::log(unsigned lineNumber, const std::string& file) {
  log(ss->str(), lineNumber, file);
  ss->clear();
}

void Logger::logError(unsigned lineNumber, const std::string& file, bool safe) {
  logError(ss->str(), lineNumber, file, safe);
  ss->clear();
}

void Logger::changelog(unsigned lineNumber, const std::string& file) {
  changelog(ss->str(), lineNumber, file);
  ss->clear();
}
