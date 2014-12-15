
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

Configuration::Configuration(int argc, char* argv[]) {
  init_conf(this);

  // FIXME: Actually parse command line options for
  // configuration and other information.

  // Build the module path.
  module_path = get_env_module_path();
}

const Configuration&
config() { return *conf_; }

} // namespace steve
