#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include "engge/System/Logger.hpp"

namespace ng {
Logger::Logger() {
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::trace);
  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt", true);
  file_sink->set_level(spdlog::level::trace);
  auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_st>();
  dist_sink->add_sink(console_sink);
  dist_sink->add_sink(file_sink);
  m_out = std::make_shared<spdlog::logger>("log", dist_sink);
  m_out->set_level(spdlog::level::trace);
}
} // namespace ng
