// A second shared library, as an engine module links wlib: it looks up loggers in spdlog's
// registry, which must be the one wlib's create_logger() registers them in. In shared builds spdlog
// is a shared library of its own (conanfile.py), so the registry exists once in the process.

#include <spdlog/spdlog.h>

#ifdef _WIN32
#  define WLIB_REGISTRY_MODULE_EXPORT __declspec(dllexport)
#else
#  define WLIB_REGISTRY_MODULE_EXPORT __attribute__((visibility("default")))
#endif

extern "C" WLIB_REGISTRY_MODULE_EXPORT bool wlib_registry_module_has_logger(char const* name)
{
  return spdlog::get(name) != nullptr;
}

extern "C" WLIB_REGISTRY_MODULE_EXPORT bool wlib_registry_module_default_is(char const* name)
{
  return spdlog::default_logger_raw() != nullptr and spdlog::default_logger_raw()->name() == name;
}
