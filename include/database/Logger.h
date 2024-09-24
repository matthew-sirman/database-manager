//
// Created by alistair on 30/08/2024.
//

#ifndef DATABASE_MANAGER_LOGGER_H
#define DATABASE_MANAGER_LOGGER_H

#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>

#define lockLog                                                              \
  if (std::unique_ptr<std::stringstream, void (*)(std::stringstream *)> ss = \
          Logger::writeStream())

/// <summary>
/// A static class that log from anywhere.
/// </summary>
class Logger {
 public:
  /// <summary>
  /// Logs the provided string to the error stream.
  /// If a line number and file are included, these are reported at the start of
  /// the log, otherwise only the log is reported.
  /// </summary>
  /// <param name="e">The error to log</param>
  /// <param name="lineNumber">The line number this error was reported.</param>
  /// <param name="file">The file wherre this error was reported.</param>
  /// <param name="safe">True if this is a non fatal error and the program
  /// should continue, false if this is a fatal error and the program should
  /// quit.</param>
  static void logError(const std::string &e, unsigned lineNumber = -1,
                       const std::string &file = "", bool safe = true);

  /// <summary>
  /// Sets the stream errors are written too.
  /// </summary>
  /// <param name="errStream">The new error stream.</param>
  static void setErrStream(std::ostream *errStream);

  /// <summary>
  /// Logs the given message to the log stream.
  /// If a line number and file are included, these are reported at the start of
  /// the log, otherwise only the log is reported.
  /// </summary>
  /// <param name="e">The message to be logged.</param>
  /// <param name="lineNumber">The line number this log was raised from.</param>
  /// <param name="file">The file this log was raised from.</param>
  static void log(const std::string &e, unsigned lineNumber = -1,
                  const std::string &file = "");

  /// <summary>
  /// Sets the stream to log to.
  /// </summary>
  /// <param name="logStream">The new log stream.</param>
  static void setLogStream(std::ostream *logStream);

  /// <summary>
  /// Logs a change in the changelog.
  /// If a line number and file are included, these are reported at the start of
  /// the log, otherwise only the log is reported.
  /// </summary>
  /// <param name="e">The changelog message to write.</param>
  /// <param name="lineNumber">The line this change was raised from.</param>
  /// <param name="file">The file this change was raised from.</param>
  static void changelog(const std::string &e, unsigned lineNumber = -1,
                        const std::string &file = "");

  /// <summary>
  /// Sets the changelog stream.
  /// </summary>
  /// <param name="changelogStream">The new changelog stream.</param>
  static void setChangelogStream(std::ostream *changelogStream);

  /// <summary>
  /// Locks the stringstream and returns a unique pointer to it that, when
  /// deleted, unlocks the stream. Macro lockLog is designed to be used with
  /// this, to handle the lifespan effectively.
  /// </summary>
  /// <returns>Unique pointer to write stream that on deletion will unlock the
  /// stringstream.</returns>
  static std::unique_ptr<std::stringstream, void (*)(std::stringstream *)>
  writeStream();

  /// <summary>
  /// Writes the content of the stringstream to the log stream.
  /// </summary>
  /// <param name="lineNumber">The line number the stream was commited
  /// from.</param> <param name="file">The file the stream was commited
  /// from.</param>
  static void log(unsigned lineNumber = -1, const std::string &file = "");

  /// <summary>
  /// Writes the content of the stringstream to the error stream.
  /// </summary>
  /// <param name="lineNumber">The line number the stream was commited
  /// from.</param> <param name="file">The file the stream was commited
  /// from.</param> <param name="safe">Represents whether this error is fatal or
  /// not. True if the error is non fatal and the program can continue, false if
  /// the error is fatal and the program must exit.</param>
  static void logError(unsigned lineNumber = -1, const std::string &file = "",
                       bool safe = true);

  /// <summary>
  /// Writes the content of the stringstream to the changelog stream.
  /// </summary>
  /// <param name="lineNumber">The line number the stream was commited
  /// from.</param> <param name="file">The file the stream was commited
  /// from.</param>
  static void changelog(unsigned lineNumber = -1, const std::string &file = "");

 private:
  Logger();
  static std::string timestamp();

  static std::ostream *errStream;
  static std::mutex errStreamLock;
  static std::ostream *logStream;
  static std::mutex logStreamLock;
  static std::ostream *changelogStream;
  static std::mutex changelogStreamLock;
  static std::stringstream *ss;
  static std::mutex streamLock;
};

#endif  // DATABASE_MANAGER_LOGGER_H
