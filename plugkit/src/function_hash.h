/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line:
 * /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gperf
 * -t -LANSI-C -G --output-file=plugkit/src/function_hash.h
 * plugkit/src/function.keys  */
/* Computed positions: -k'3,12' */

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

#line 3 "plugkit/src/function.keys"
struct Functions {
  Functions(const char *name = nullptr, void *func = nullptr)
      : name(name), func(func) {}
  const char *name;
  void *func;
};

#define TOTAL_KEYWORDS 33
#define MIN_WORD_LENGTH 10
#define MAX_WORD_LENGTH 23
#define MIN_HASH_VALUE 10
#define MAX_HASH_VALUE 52
/* maximum key range = 43, duplicates = 0 */

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
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 10, 0,  53, 30, 53, 53, 53,
      53, 25, 53, 53, 0,  53, 30, 15, 53, 53, 53, 5,  53, 20, 20, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 10, 20, 20, 53, 53, 53, 5,  53, 10, 53, 15,
      53, 53, 15, 53, 53, 53, 5,  10, 0,  53, 53, 53, 53, 0,  53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
      53, 53, 53, 53};
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
  case 4:
  case 3:
    hval += asso_values[(unsigned char)str[2]];
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
    {""},
    {""},
#line 16 "plugkit/src/function.keys"
    {"Layer_attr", reinterpret_cast<void *>(&plugkit::Layer_attr)},
    {""},
    {""},
#line 14 "plugkit/src/function.keys"
    {"Layer_addAttr", reinterpret_cast<void *>(&plugkit::Layer_addAttr)},
#line 12 "plugkit/src/function.keys"
    {"Layer_addLayer", reinterpret_cast<void *>(&plugkit::Layer_addLayer)},
#line 42 "plugkit/src/function.keys"
    {"Logger_log", reinterpret_cast<void *>(&plugkit::Logger_log)},
#line 17 "plugkit/src/function.keys"
    {"Layer_addPayload", reinterpret_cast<void *>(&plugkit::Layer_addPayload)},
#line 21 "plugkit/src/function.keys"
    {"Layer_addTag", reinterpret_cast<void *>(&plugkit::Layer_addTag)},
#line 15 "plugkit/src/function.keys"
    {"Layer_addAttrAlias",
     reinterpret_cast<void *>(&plugkit::Layer_addAttrAlias)},
#line 19 "plugkit/src/function.keys"
    {"Layer_addError", reinterpret_cast<void *>(&plugkit::Layer_addError)},
#line 26 "plugkit/src/function.keys"
    {"Variant_setBool", reinterpret_cast<void *>(&plugkit::Variant_setBool)},
#line 22 "plugkit/src/function.keys"
    {"Payload_addSlice", reinterpret_cast<void *>(&plugkit::Payload_addSlice)},
#line 20 "plugkit/src/function.keys"
    {"Layer_errors", reinterpret_cast<void *>(&plugkit::Layer_errors)},
#line 40 "plugkit/src/function.keys"
    {"Error_message", reinterpret_cast<void *>(&plugkit::Error_message)},
#line 18 "plugkit/src/function.keys"
    {"Layer_payloads", reinterpret_cast<void *>(&plugkit::Layer_payloads)},
#line 24 "plugkit/src/function.keys"
    {"Payload_addAttr", reinterpret_cast<void *>(&plugkit::Payload_addAttr)},
#line 33 "plugkit/src/function.keys"
    {"Variant_setSlice", reinterpret_cast<void *>(&plugkit::Variant_setSlice)},
#line 31 "plugkit/src/function.keys"
    {"Variant_setString",
     reinterpret_cast<void *>(&plugkit::Variant_setString)},
    {""},
#line 30 "plugkit/src/function.keys"
    {"Variant_string", reinterpret_cast<void *>(&plugkit::Variant_string)},
#line 32 "plugkit/src/function.keys"
    {"Variant_setStringRef",
     reinterpret_cast<void *>(&plugkit::Variant_setStringRef)},
#line 41 "plugkit/src/function.keys"
    {"Error_setMessage", reinterpret_cast<void *>(&plugkit::Error_setMessage)},
#line 39 "plugkit/src/function.keys"
    {"Token_string", reinterpret_cast<void *>(&plugkit::Token_string)},
#line 34 "plugkit/src/function.keys"
    {"Variant_arrayValue",
     reinterpret_cast<void *>(&plugkit::Variant_arrayValue)},
#line 23 "plugkit/src/function.keys"
    {"Payload_slices", reinterpret_cast<void *>(&plugkit::Payload_slices)},
    {""},
#line 35 "plugkit/src/function.keys"
    {"Variant_arrayValueRef",
     reinterpret_cast<void *>(&plugkit::Variant_arrayValueRef)},
#line 13 "plugkit/src/function.keys"
    {"Layer_addSubLayer",
     reinterpret_cast<void *>(&plugkit::Layer_addSubLayer)},
#line 11 "plugkit/src/function.keys"
    {"Context_addLayerLinkage",
     reinterpret_cast<void *>(&plugkit::Context_addLayerLinkage)},
#line 38 "plugkit/src/function.keys"
    {"Token_literal_", reinterpret_cast<void *>(&plugkit::Token_literal_)},
    {""},
#line 36 "plugkit/src/function.keys"
    {"Variant_mapValue", reinterpret_cast<void *>(&plugkit::Variant_mapValue)},
#line 28 "plugkit/src/function.keys"
    {"Variant_setUint64",
     reinterpret_cast<void *>(&plugkit::Variant_setUint64)},
    {""},
#line 37 "plugkit/src/function.keys"
    {"Variant_mapValueRef",
     reinterpret_cast<void *>(&plugkit::Variant_mapValueRef)},
    {""},
#line 27 "plugkit/src/function.keys"
    {"Variant_setInt64", reinterpret_cast<void *>(&plugkit::Variant_setInt64)},
#line 10 "plugkit/src/function.keys"
    {"Context_getOption",
     reinterpret_cast<void *>(&plugkit::Context_getOption)},
    {""},
#line 25 "plugkit/src/function.keys"
    {"Variant_setNil", reinterpret_cast<void *>(&plugkit::Variant_setNil)},
    {""},
    {""},
#line 29 "plugkit/src/function.keys"
    {"Variant_setDouble",
     reinterpret_cast<void *>(&plugkit::Variant_setDouble)}};

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
