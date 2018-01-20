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
  static void *resolve(const char *name);

private:
  void *lib;
  std::string mError;
};

template <class T>
T ModuleLoader::load(const char *name) {
#if defined(PLUGKIT_OS_LINUX) || defined(PLUGKIT_OS_MAC)
  return reinterpret_cast<T>(dlsym(lib, name));
#elif defined(PLUGKIT_OS_WIN)
  return reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(lib), name));
#endif
}

} // namespace plugkit

#endif
