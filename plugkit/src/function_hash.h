/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line:
 * /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gperf
 * -t -LANSI-C -G --output-file=plugkit/src/function_hash.h
 * plugkit/src/function.keys  */
/* Computed positions: -k'7,10' */

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

#define TOTAL_KEYWORDS 16
#define MIN_WORD_LENGTH 10
#define MAX_WORD_LENGTH 23
#define MIN_HASH_VALUE 10
#define MAX_HASH_VALUE 34
/* maximum key range = 25, duplicates = 0 */

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
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 0,  35, 35, 35, 20, 35, 35,
      35, 35, 35, 35, 15, 35, 35, 35, 5,  35, 35, 35, 5,  35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 5,  35, 0,  35, 35, 0,  5,  35, 5,  35, 0,  35, 35,
      0,  35, 35, 35, 10, 35, 0,  0,  0,  35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
      35, 35, 35, 35};
  return len + asso_values[(unsigned char)str[9]] +
         asso_values[(unsigned char)str[6]];
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
#line 15 "plugkit/src/function.keys"
    {"Layer_attr", reinterpret_cast<void *>(&plugkit::Layer_attr)},
    {""},
#line 24 "plugkit/src/function.keys"
    {"Token_string", reinterpret_cast<void *>(&plugkit::Token_string)},
#line 13 "plugkit/src/function.keys"
    {"Layer_addAttr", reinterpret_cast<void *>(&plugkit::Layer_addAttr)},
#line 21 "plugkit/src/function.keys"
    {"Payload_slices", reinterpret_cast<void *>(&plugkit::Payload_slices)},
#line 22 "plugkit/src/function.keys"
    {"Payload_addAttr", reinterpret_cast<void *>(&plugkit::Payload_addAttr)},
#line 20 "plugkit/src/function.keys"
    {"Payload_addSlice", reinterpret_cast<void *>(&plugkit::Payload_addSlice)},
#line 19 "plugkit/src/function.keys"
    {"Layer_addTag", reinterpret_cast<void *>(&plugkit::Layer_addTag)},
#line 14 "plugkit/src/function.keys"
    {"Layer_addAttrAlias",
     reinterpret_cast<void *>(&plugkit::Layer_addAttrAlias)},
#line 23 "plugkit/src/function.keys"
    {"Token_literal_", reinterpret_cast<void *>(&plugkit::Token_literal_)},
#line 25 "plugkit/src/function.keys"
    {"Logger_log", reinterpret_cast<void *>(&plugkit::Logger_log)},
#line 16 "plugkit/src/function.keys"
    {"Layer_addPayload", reinterpret_cast<void *>(&plugkit::Layer_addPayload)},
#line 10 "plugkit/src/function.keys"
    {"Context_getOption",
     reinterpret_cast<void *>(&plugkit::Context_getOption)},
#line 11 "plugkit/src/function.keys"
    {"Context_addLayerLinkage",
     reinterpret_cast<void *>(&plugkit::Context_addLayerLinkage)},
#line 17 "plugkit/src/function.keys"
    {"Layer_payloads", reinterpret_cast<void *>(&plugkit::Layer_payloads)},
    {""},
    {""},
    {""},
    {""},
#line 12 "plugkit/src/function.keys"
    {"Layer_addLayer", reinterpret_cast<void *>(&plugkit::Layer_addLayer)},
    {""},
    {""},
    {""},
    {""},
#line 18 "plugkit/src/function.keys"
    {"Layer_addError", reinterpret_cast<void *>(&plugkit::Layer_addError)}};

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
