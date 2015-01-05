
#include <steve/Config.hpp>
#include <steve/Debug.hpp>

#include <cstdlib>

namespace steve {

namespace {

// Initialization

// Configuration initialization.
Configuration* conf_ = nullptr;

void
init_conf(Configuration* c) {
  steve_assert(not conf_, "configuration already initialized");
  conf_ = c;
}

void
reset_conf(Configuration* c) {
  steve_assert(conf_, "configuration not initialized");
  steve_assert(conf_ == c, "reconfiguration error");
  conf_ = nullptr;
}

// Environment variables

// Get the module search path from the environment.
Path_list
get_env_module_path() {
  const char* var = getenv("STEVE_MODULE_PATH");
  if (not var)
    return {};

  // FIXME: Refactor as an algorithm.
  Path_list paths;
  const char* first = var;
  const char* last = std::strchr(var, ':');
  while (last) {
    paths.emplace_back(first, last);
    first = last + 1;
    last = first;
  }
  paths.emplace_back(first);

  return paths;
}

} // namespace

Configuration::Configuration() {
  init_conf(this);
  module_path = get_env_module_path();
}

Configuration::~Configuration() {
  reset_conf(this);
}

// Returns the global configuration object.
Configuration&
config() { return *conf_; }

} // namespace steve
