/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line:
 * /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gperf
 * -t -LANSI-C -G --output-file=plugkit/src/function_hash.h
 * plugkit/src/function.keys  */
/* Computed positions: -k'5,12' */

#if !(                                                                         \
    (' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) && ('%' == 37) && \
    ('&' == 38) && ('\'' == 39) && ('(' == 40) && (')' == 41) &&               \
    ('*' == 42) && ('+' == 43) && (',' == 44) && ('-' == 45) && ('.' == 46) && \
    ('/' == 47) && ('0' == 48) && ('1' == 49) && ('2' == 50) && ('3' == 51) && \
    ('4' == 52) && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) && \
    ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) && ('=' == 61) && \
    ('>' == 62) && ('?' == 63) && ('A' == 65) && ('B' == 66) && ('C' == 67) && \
    ('D' == 68) && ('E' == 69) && ('F' == 70) && ('G' == 71) && ('H' == 72) && \
    ('I' == 73) && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) && \
    ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) && ('R' == 82) && \
    ('S' == 83) && ('T' == 84) && ('U' == 85) && ('V' == 86) && ('W' == 87) && \
    ('X' == 88) && ('Y' == 89) && ('Z' == 90) && ('[' == 91) &&                \
    ('\\' == 92) && (']' == 93) && ('^' == 94) && ('_' == 95) &&               \
    ('a' == 97) && ('b' == 98) && ('c' == 99) && ('d' == 100) &&               \
    ('e' == 101) && ('f' == 102) && ('g' == 103) && ('h' == 104) &&            \
    ('i' == 105) && ('j' == 106) && ('k' == 107) && ('l' == 108) &&            \
    ('m' == 109) && ('n' == 110) && ('o' == 111) && ('p' == 112) &&            \
    ('q' == 113) && ('r' == 114) && ('s' == 115) && ('t' == 116) &&            \
    ('u' == 117) && ('v' == 118) && ('w' == 119) && ('x' == 120) &&            \
    ('y' == 121) && ('z' == 122) && ('{' == 123) && ('|' == 124) &&            \
    ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error                                                                         \
    "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "plugkit/src/function.keys"

#include "attr.h"
#include "context.h"
#include "layer.h"
#include "logger.h"
#include "payload.h"
#include "variant.h"
#line 9 "plugkit/src/function.keys"
struct Functions {
  Functions(const char *name = nullptr, void *func = nullptr)
      : name(name), func(func) {}
  const char *name;
  void *func;
};

#define TOTAL_KEYWORDS 47
#define MIN_WORD_LENGTH 8
#define MAX_WORD_LENGTH 21
#define MIN_HASH_VALUE 8
#define MAX_HASH_VALUE 74
/* maximum key range = 67, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
    static unsigned int
    hash(register const char *str, register unsigned int len) {
  static unsigned char asso_values[] = {
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 5,  75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 30, 10, 75, 40, 75, 75, 75,
      75, 20, 75, 75, 75, 75, 50, 15, 75, 75, 15, 0,  5,  30, 40, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 10, 35, 40, 20, 30, 75, 10, 75, 40, 75, 75,
      25, 75, 15, 0,  75, 75, 0,  75, 20, 75, 75, 0,  75, 5,  75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
      75, 75, 75, 75};
  register unsigned int hval = len;

  switch (hval) {
  default:
    hval += asso_values[(unsigned char)str[11]];
  /*FALLTHROUGH*/
  case 11:
  case 10:
  case 9:
  case 8:
  case 7:
  case 6:
  case 5:
    hval += asso_values[(unsigned char)str[4]];
    break;
  }
  return hval;
}

static struct Functions wordlist[] = {
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
#line 17 "plugkit/src/function.keys"
    {"Layer_id", reinterpret_cast<void *>(&plugkit::Layer_id)},
#line 33 "plugkit/src/function.keys"
    {"Log_warn_", reinterpret_cast<void *>(&plugkit::Log_warn_)},
#line 28 "plugkit/src/function.keys"
    {"Layer_attr", reinterpret_cast<void *>(&plugkit::Layer_attr)},
#line 19 "plugkit/src/function.keys"
    {"Layer_range", reinterpret_cast<void *>(&plugkit::Layer_range)},
#line 22 "plugkit/src/function.keys"
    {"Layer_worker", reinterpret_cast<void *>(&plugkit::Layer_worker)},
    {""},
    {""},
#line 23 "plugkit/src/function.keys"
    {"Layer_setWorker", reinterpret_cast<void *>(&plugkit::Layer_setWorker)},
#line 36 "plugkit/src/function.keys"
    {"Payload_addSlice", reinterpret_cast<void *>(&plugkit::Payload_addSlice)},
    {""},
    {""},
#line 25 "plugkit/src/function.keys"
    {"Layer_addLayer", reinterpret_cast<void *>(&plugkit::Layer_addLayer)},
#line 40 "plugkit/src/function.keys"
    {"Payload_setType", reinterpret_cast<void *>(&plugkit::Payload_setType)},
#line 29 "plugkit/src/function.keys"
    {"Layer_addPayload", reinterpret_cast<void *>(&plugkit::Layer_addPayload)},
#line 31 "plugkit/src/function.keys"
    {"Layer_addTag", reinterpret_cast<void *>(&plugkit::Layer_addTag)},
#line 41 "plugkit/src/function.keys"
    {"Payload_range", reinterpret_cast<void *>(&plugkit::Payload_range)},
#line 30 "plugkit/src/function.keys"
    {"Layer_payloads", reinterpret_cast<void *>(&plugkit::Layer_payloads)},
#line 61 "plugkit/src/function.keys"
    {"Token_join", reinterpret_cast<void *>(&plugkit::Token_join)},
#line 55 "plugkit/src/function.keys"
    {"Variant_setSlice", reinterpret_cast<void *>(&plugkit::Variant_setSlice)},
#line 53 "plugkit/src/function.keys"
    {"Variant_setString",
     reinterpret_cast<void *>(&plugkit::Variant_setString)},
#line 46 "plugkit/src/function.keys"
    {"Variant_int32", reinterpret_cast<void *>(&plugkit::Variant_int32)},
#line 20 "plugkit/src/function.keys"
    {"Layer_setRange", reinterpret_cast<void *>(&plugkit::Layer_setRange)},
#line 32 "plugkit/src/function.keys"
    {"Log_debug_", reinterpret_cast<void *>(&plugkit::Log_debug_)},
#line 42 "plugkit/src/function.keys"
    {"Payload_setRange", reinterpret_cast<void *>(&plugkit::Payload_setRange)},
#line 24 "plugkit/src/function.keys"
    {"Layer_parent", reinterpret_cast<void *>(&plugkit::Layer_parent)},
#line 27 "plugkit/src/function.keys"
    {"Layer_addAttr", reinterpret_cast<void *>(&plugkit::Layer_addAttr)},
#line 21 "plugkit/src/function.keys"
    {"Layer_setConfidence",
     reinterpret_cast<void *>(&plugkit::Layer_setConfidence)},
#line 45 "plugkit/src/function.keys"
    {"Variant_setBool", reinterpret_cast<void *>(&plugkit::Variant_setBool)},
#line 18 "plugkit/src/function.keys"
    {"Layer_confidence", reinterpret_cast<void *>(&plugkit::Layer_confidence)},
#line 62 "plugkit/src/function.keys"
    {"Token_string", reinterpret_cast<void *>(&plugkit::Token_string)},
#line 56 "plugkit/src/function.keys"
    {"Variant_arrayValue",
     reinterpret_cast<void *>(&plugkit::Variant_arrayValue)},
#line 60 "plugkit/src/function.keys"
    {"Token_literal_", reinterpret_cast<void *>(&plugkit::Token_literal_)},
#line 35 "plugkit/src/function.keys"
    {"Log_error_", reinterpret_cast<void *>(&plugkit::Log_error_)},
#line 57 "plugkit/src/function.keys"
    {"Variant_arrayValueRef",
     reinterpret_cast<void *>(&plugkit::Variant_arrayValueRef)},
#line 39 "plugkit/src/function.keys"
    {"Payload_type", reinterpret_cast<void *>(&plugkit::Payload_type)},
    {""},
#line 48 "plugkit/src/function.keys"
    {"Variant_uint32", reinterpret_cast<void *>(&plugkit::Variant_uint32)},
#line 38 "plugkit/src/function.keys"
    {"Payload_addAttr", reinterpret_cast<void *>(&plugkit::Payload_addAttr)},
#line 47 "plugkit/src/function.keys"
    {"Variant_setInt32", reinterpret_cast<void *>(&plugkit::Variant_setInt32)},
#line 44 "plugkit/src/function.keys"
    {"Variant_bool", reinterpret_cast<void *>(&plugkit::Variant_bool)},
    {""},
#line 34 "plugkit/src/function.keys"
    {"Log_info_", reinterpret_cast<void *>(&plugkit::Log_info_)},
    {""},
    {""},
#line 26 "plugkit/src/function.keys"
    {"Layer_addSubLayer",
     reinterpret_cast<void *>(&plugkit::Layer_addSubLayer)},
    {""},
#line 37 "plugkit/src/function.keys"
    {"Payload_slices", reinterpret_cast<void *>(&plugkit::Payload_slices)},
    {""},
    {""},
#line 49 "plugkit/src/function.keys"
    {"Variant_setUint32",
     reinterpret_cast<void *>(&plugkit::Variant_setUint32)},
    {""},
#line 50 "plugkit/src/function.keys"
    {"Variant_double", reinterpret_cast<void *>(&plugkit::Variant_double)},
    {""},
    {""},
#line 16 "plugkit/src/function.keys"
    {"Context_getOption",
     reinterpret_cast<void *>(&plugkit::Context_getOption)},
#line 54 "plugkit/src/function.keys"
    {"Variant_slice", reinterpret_cast<void *>(&plugkit::Variant_slice)},
#line 52 "plugkit/src/function.keys"
    {"Variant_string", reinterpret_cast<void *>(&plugkit::Variant_string)},
    {""},
#line 58 "plugkit/src/function.keys"
    {"Variant_mapValue", reinterpret_cast<void *>(&plugkit::Variant_mapValue)},
#line 51 "plugkit/src/function.keys"
    {"Variant_setDouble",
     reinterpret_cast<void *>(&plugkit::Variant_setDouble)},
    {""},
#line 59 "plugkit/src/function.keys"
    {"Variant_mapValueRef",
     reinterpret_cast<void *>(&plugkit::Variant_mapValueRef)},
    {""},
    {""},
    {""},
    {""},
#line 43 "plugkit/src/function.keys"
    {"Variant_setNil", reinterpret_cast<void *>(&plugkit::Variant_setNil)}};

struct Functions *in_word_set(register const char *str,
                              register unsigned int len) {
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
    unsigned int key = hash(str, len);

    if (key <= MAX_HASH_VALUE) {
      register const char *s = wordlist[key].name;

      if (*str == *s && !strcmp(str + 1, s + 1))
        return &wordlist[key];
    }
  }
  return 0;
}
