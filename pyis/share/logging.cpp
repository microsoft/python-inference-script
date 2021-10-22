// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifdef USE_SPDLOG

#include "pyis/share/logging.h"

#include "pyis/share/file_system.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace pyis {

Logger logger;

Logger::Logger(bool std_out, bool std_err, const spdlog::level::level_enum& level, const std::string& format) {
    level_ = level;
    format_ = format;
    AddConsole(std_out, std_err);
}

Logger::Logger(const std::string& dir, const std::string& file, const spdlog::level::level_enum& level,
               const std::string& format) {
    level_ = level;
    format_ = format;
    AddFile(dir, file);
}

void Logger::AddConsole(bool std_out, bool std_err) {
    if (std_out) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(level_);
        console_sink->set_pattern(this->format_);
        sinks_.push_back(console_sink);
    }

    if (std_err) {
        auto console_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        console_sink->set_level(level_);
        console_sink->set_pattern(this->format_);
        sinks_.push_back(console_sink);
    }
    inner_logger_ = std::make_shared<spdlog::logger>("pyis", sinks_.begin(), sinks_.end());
    inner_logger_->set_level(level_);
    inner_logger_->set_pattern(this->format_);
}

void Logger::AddFile(const std::string& dir, const std::string& file) {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(FileSystem::join_path({dir, file}), true);
    file_sink->set_level(level_);
    file_sink->set_pattern(this->format_);
    sinks_.push_back(file_sink);
    inner_logger_ = std::make_shared<spdlog::logger>("pyis", sinks_.begin(), sinks_.end());
    inner_logger_->set_level(level_);
    inner_logger_->set_pattern(this->format_);
}

void Logger::Debug(const char* msg) { inner_logger_->debug(msg); }

void Logger::Info(const char* msg) { inner_logger_->info(msg); }

void Logger::Warn(const char* msg) { inner_logger_->warn(msg); }

void Logger::Error(const char* msg) { inner_logger_->error(msg); }

}  // namespace pyis

#endif
