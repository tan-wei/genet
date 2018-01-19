#ifndef PLUGKIT_MODULE_LOADER_HPP
#define PLUGKIT_MODULE_LOADER_HPP

#include <string>

#if defined(PLUGKIT_OS_LINUX) || defined(PLUGKIT_OS_MAC)
#include <dlfcn.h>
#elif defined(PLUGKIT_OS_WIN)
#include <windows.h>
#endif

namespace plugkit {

class ModuleLoader {
public:
  ModuleLoader(const std::string &path);
  template <class V>
  V load(const char *name);
  const std::string error() const;

private:
  void *lib;
  std::string mError;
};

inline ModuleLoader::ModuleLoader(const std::string &path) : lib(nullptr) {
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

template <class T>
T ModuleLoader::load(const char *name) {
#if defined(PLUGKIT_OS_LINUX) || defined(PLUGKIT_OS_MAC)
  return reinterpret_cast<T>(dlsym(lib, name));
#elif defined(PLUGKIT_OS_WIN)
  return reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(lib), name));
#endif
}

inline const std::string ModuleLoader::error() const { return mError; }

} // namespace plugkit

#endif
