#ifdef PLUGKIT_OS_WIN
#ifdef PLUGKIT_DLL_EXPORT
#define PLUGKIT_EXPORT __declspec(dllexport)
#else
#define PLUGKIT_EXPORT __declspec(dllimport)
#endif
#else
#define PLUGKIT_EXPORT
#endif

#ifdef PLUGKIT_OS_WIN
#define PLUGKIT_MODULE_EXPORT __declspec(dllexport)
#else
#define PLUGKIT_MODULE_EXPORT
#endif


#ifdef __cplusplus
#define PLUGKIT_NAMESPACE_BEGIN                                                \
  extern "C" {                                                                 \
  namespace plugkit {
#define PLUGKIT_NAMESPACE_END                                                  \
  }                                                                            \
  }
#else
#define PLUGKIT_NAMESPACE_BEGIN
#define PLUGKIT_NAMESPACE_END
#endif
