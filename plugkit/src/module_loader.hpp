#ifndef PLUGKIT_MODULE_LOADER_HPP
#define PLUGKIT_MODULE_LOADER_HPP

#include <string>

#if defined(PLUGKIT_OS_LINUX) || defined(PLUGKIT_OS_MAC)
#include <dlfcn.h>
#elif defined(PLUGKIT_OS_WIN)
#include <windows.h>
#endif

namespace plugkit {

template <class T>
class ModuleLoader {
public:
  bool load(T *target, const std::string &path);
  const std::string error() const;

private:
  std::string mError;
};

template <class T>
bool ModuleLoader<T>::load(T *target, const std::string &path) {
#if defined(PLUGKIT_OS_LINUX) || defined(PLUGKIT_OS_MAC)
  void *lib = dlopen(path.c_str(), RTLD_LOCAL | RTLD_LAZY);
  if (!lib) {
    mError = dlerror();
    return false;
  }
  void *init = dlsym(lib, "plugkit_module_init");
  if (!init) {
    mError = dlerror();
    return false;
  }
#elif defined(PLUGKIT_OS_WIN)
  HINSTANCE lib = LoadLibrary(path.c_str());
  if (!lib) {
    mError = "LoadLibrary() failed";
    return false;
  }
  FARPROC init = GetProcAddress(lib, "plugkit_module_init");
  if (!init) {
    mError = "GetProcAddress() failed";
    return false;
  }
#endif
  reinterpret_cast<void (*)(T *)>(init)(target);
  return true;
}

template <class T>
const std::string ModuleLoader<T>::error() const {
  return mError;
}

} // namespace plugkit

#endif
