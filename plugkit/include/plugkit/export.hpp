#ifdef PLUGKIT_OS_WIN
#ifdef PLUGKIT_DLL_EXPORT
#define PLUGKIT_EXPORT __declspec(dllexport)
#else
#define PLUGKIT_EXPORT __declspec(dllimport)
#endif
#else
#define PLUGKIT_EXPORT
#endif
