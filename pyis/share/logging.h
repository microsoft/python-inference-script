// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <vector>

#ifdef USE_SPDLOG
#include "spdlog/spdlog.h"
#else
#include <cstdio>
#endif

namespace pyis {

#ifdef USE_SPDLOG

class Logger {
  public:
    explicit Logger(bool std_out = false, bool std_err = true,
                    const spdlog::level::level_enum& level = spdlog::level::level_enum::info,
                    const std::string& format = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");
    Logger(const std::string& dir, const std::string& file,
           const spdlog::level::level_enum& level = spdlog::level::level_enum::info,
           const std::string& format = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");

    template <typename... Rest>
    void Debug(const char* fmt, const Rest&... rest) {
        inner_logger_->debug(fmt, rest...);
    }

    template <typename... Rest>
    void Info(const char* fmt, const Rest&... rest) {
        inner_logger_->info(fmt, rest...);
    }

    template <typename... Rest>
    void Warn(const char* fmt, const Rest&... rest) {
        inner_logger_->warn(fmt, rest...);
    }

    template <typename... Rest>
    void Error(const char* fmt, const Rest&... rest) {
        inner_logger_->error(fmt, rest...);
    }

    void Debug(const char* msg);
    void Info(const char* msg);
    void Warn(const char* msg);
    void Error(const char* msg);

  private:
    void AddConsole(bool std_out = false, bool std_err = true);
    void AddFile(const std::string& dir, const std::string& file = "pyis.log");

    std::string format_;
    std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks_;
    std::shared_ptr<spdlog::logger> inner_logger_;
    spdlog::level::level_enum level_;
};

extern Logger logger;
#define LOG_DEBUG(...) logger.Debug(__VA_ARGS__)
#define LOG_INFO(...) logger.Info(__VA_ARGS__)
#define LOG_WARN(...) logger.Warn(__VA_ARGS__)
#define LOG_ERROR(...) logger.Error(__VA_ARGS__)

#else

#define LOG_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#define LOG_INFO(...) fprintf(stderr, __VA_ARGS__)
#define LOG_WARN(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)

#endif

}  // namespace pyis
