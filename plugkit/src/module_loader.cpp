#include "module_loader.hpp"
#include "attr.hpp"
#include "context.hpp"
#include "error.hpp"
#include "layer.hpp"
#include "payload.hpp"
#include "variant.hpp"

namespace {

#define register
#include "function_hash.h"
#undef register

} // namespace

namespace plugkit {

ModuleLoader::ModuleLoader(const std::string &path) : lib(nullptr) {
#if defined(PLUGKIT_OS_LINUX) || defined(PLUGKIT_OS_MAC)
  lib = dlopen(path.c_str(), RTLD_LOCAL | RTLD_LAZY);
  if (!lib) {
    mError = dlerror();
  }
#elif defined(PLUGKIT_OS_WIN)
  lib = LoadLibrary(path.c_str());
  if (!lib) {
    mError = "LoadLibrary() failed";
  }
#endif
}

const std::string ModuleLoader::error() const { return mError; }

void *ModuleLoader::resolve(const char *name) {
  const auto &data = in_word_set(name, strlen(name));
  if (data) {
    return data->func;
  }
  return nullptr;
}

} // namespace plugkit
