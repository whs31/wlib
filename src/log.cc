#include <w/common/log.h>

#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
#include <string_view>

#include <spdlog/async.h>
#include <spdlog/sinks/callback_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace
{
  [[nodiscard]] constexpr spdlog::level::level_enum default_log_level() noexcept
  {
#ifdef W_DEBUG
    return spdlog::level::debug;
#else
    return spdlog::level::info;
#endif
  }

  struct LogState
  {
    std::shared_ptr<spdlog::sinks::dist_sink_mt>
      dynamic_sink_router = std::make_shared<spdlog::sinks::dist_sink_mt>();
  };

  [[nodiscard]] LogState& log_state()
  {
    static auto state = LogState();
    return state;
  }

  [[nodiscard]] bool has_target(
    w::common::LogTarget const target,
    w::common::LogTarget const flag
  ) noexcept
  {
    return (target & flag) == flag;
  }

  [[nodiscard]] std::size_t file_size_bytes(float const max_file_size_mb)
  {
    if(not std::isfinite(max_file_size_mb) or max_file_size_mb <= 0.0f)
      throw w::common::LogError("max_file_size_mb must be finite and greater than zero");

    auto constexpr bytes_per_mebibyte = 1'024.0 * 1'024.0;
    auto const bytes = static_cast<double>(max_file_size_mb) * bytes_per_mebibyte;
    if(bytes > static_cast<double>(std::numeric_limits<std::size_t>::max()))
      throw w::common::LogError("max_file_size_mb is too large");

    return static_cast<std::size_t>(bytes);
  }

  void validate_config(w::common::LoggerConfig const& config)
  {
    auto constexpr known_targets = std::to_underlying(w::common::LogTarget::Stdout)
                                 | std::to_underlying(w::common::LogTarget::File);
    auto const target = std::to_underlying(config.target);

    if(config.name.empty())
      throw w::common::LogError("logger name must not be empty");
    if(target == 0 or (target & ~known_targets) != 0)
      throw w::common::
        LogError("invalid logger target mask '{}'", static_cast<unsigned int>(target));
    for(auto const& sink : config.additional_sinks) {
      if(not sink)
        throw w::common::LogError("additional logger sinks must not be null");
    }

    if(has_target(config.target, w::common::LogTarget::File)) {
      if(not config.filename or config.filename->empty())
        throw w::common::LogError("filename must be non-empty for file target logging");
      if(config.max_files == 0)
        throw w::common::LogError("max_files must be greater than zero");
      static_cast<void>(file_size_bytes(config.max_file_size_mb));
    }
  }

}  // namespace

W_COMMON_NAMESPACE_BEGIN

spdlog::level::level_enum get_env_log_level() noexcept
{
  if(char const* env = std::getenv("LOG_LEVEL")) {
    auto const level = spdlog::level::from_str(env);
    if(level != spdlog::level::off or std::string_view {env} == "off")
      return level;
  }
  return default_log_level();
}

std::shared_ptr<spdlog::logger> create_logger(LoggerConfig const& config)
{
  validate_config(config);

  auto sinks = std::vector<spdlog::sink_ptr>();

  if(has_target(config.target, LogTarget::Stdout))
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

  if(has_target(config.target, LogTarget::File)) {
    auto const folder = config.folder.value_or(std::filesystem::current_path());
    if(not std::filesystem::exists(folder))
      std::filesystem::create_directories(folder);

    auto const base_path = std::filesystem::path(config.filename.value_or(""));
    auto const launch_filename = folder / base_path;

    sinks.push_back(
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        launch_filename.string(),
        file_size_bytes(config.max_file_size_mb),
        config.max_files
      )
    );
  }

  if(config.on_warn_or_error) {
    auto const callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>(
      [cb = config.on_warn_or_error](spdlog::details::log_msg const& msg) { cb(msg); }
    );
    callback_sink->set_level(spdlog::level::warn);
    sinks.push_back(callback_sink);
  }
  sinks.insert(sinks.end(), config.additional_sinks.begin(), config.additional_sinks.end());
  sinks.push_back(log_state().dynamic_sink_router);

  auto logger = std::shared_ptr<spdlog::logger>();
  if(config.use_async) {
    if(not spdlog::thread_pool())
      spdlog::init_thread_pool(1'024, 1);
    logger = std::make_shared<spdlog::async_logger>(
      config.name,
      sinks.begin(),
      sinks.end(),
      spdlog::thread_pool(),
      spdlog::async_overflow_policy::block
    );
  }
  else {
    logger = std::make_shared<spdlog::logger>(config.name, sinks.begin(), sinks.end());
  }

  logger->set_level(config.level);
  if(not config.pattern.empty())
    logger->set_pattern(config.pattern);
  logger->flush_on(spdlog::level::debug);

  spdlog::register_logger(logger);
  if(config.is_default)
    spdlog::set_default_logger(logger);

  logger->trace("logger '{}' initialized successfully", config.name);
  return logger;
}

void shutdown_log() { spdlog::shutdown(); }

void attach_log_sink(spdlog::sink_ptr sink)
{
  if(not sink)
    throw LogError("log sink must not be null");

  log_state().dynamic_sink_router->add_sink(std::move(sink));
}

void detach_log_sink(spdlog::sink_ptr const& sink) noexcept
{
  if(sink)
    log_state().dynamic_sink_router->remove_sink(sink);
}

W_COMMON_NAMESPACE_END
