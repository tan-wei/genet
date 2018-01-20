/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gperf -t -LANSI-C -G --output-file=plugkit/src/function_hash.h plugkit/src/function.keys  */
/* Computed positions: -k'' */

#line 1 "plugkit/src/function.keys"

#include "attr.h"
#line 4 "plugkit/src/function.keys"
struct Functions
{
  Functions(const char *name = nullptr, void *func = nullptr) : name(name), func(func) {}
  const char *name;
  void *func;
};

#define TOTAL_KEYWORDS 3
#define MIN_WORD_LENGTH 9
#define MAX_WORD_LENGTH 12
#define MIN_HASH_VALUE 9
#define MAX_HASH_VALUE 12
/* maximum key range = 4, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
/*ARGSUSED*/
static unsigned int
hash (register const char *str, register unsigned int len)
{
  return len;
}

static struct Functions wordlist[] =
  {
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 11 "plugkit/src/function.keys"
    {"Token_get", reinterpret_cast<void *>(&plugkit::Token_literal_)},
#line 13 "plugkit/src/function.keys"
    {"Token_join", reinterpret_cast<void *>(&plugkit::Token_join)},
    {""},
#line 12 "plugkit/src/function.keys"
    {"Token_string", reinterpret_cast<void *>(&plugkit::Token_string)}
  };

struct Functions *
in_word_set (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
