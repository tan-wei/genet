/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -LANSI-C -G --output-file=plugkit/src/token_hash.h
 * plugkit/src/token.keys  */
/* Computed positions: -k'2,4-5,16,$' */

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
    "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#define TOTAL_KEYWORDS 94
#define MIN_WORD_LENGTH 1
#define MAX_WORD_LENGTH 24
#define MIN_HASH_VALUE 1
#define MAX_HASH_VALUE 260
/* maximum key range = 260, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
    static unsigned int
    hash(register const char *str, register size_t len) {
  static unsigned short asso_values[] = {
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 0,   261, 261, 261, 261, 261, 261, 261, 261,
      0,   10,  261, 261, 261, 261, 261, 0,   261, 5,   261, 261, 261, 0,   261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 100, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 0,   261, 261, 261, 40,  261, 45,  5,   65,  35,  100, 65,
      70,  261, 55,  70,  95,  30,  0,   0,   25,  0,   30,  15,  0,   0,   261,
      60,  0,   261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261, 261,
      261};
  register unsigned int hval = len;

  switch (hval) {
  default:
    hval += asso_values[(unsigned char)str[15]];
  /*FALLTHROUGH*/
  case 15:
  case 14:
  case 13:
  case 12:
  case 11:
  case 10:
  case 9:
  case 8:
  case 7:
  case 6:
  case 5:
    hval += asso_values[(unsigned char)str[4]];
  /*FALLTHROUGH*/
  case 4:
    hval += asso_values[(unsigned char)str[3]];
  /*FALLTHROUGH*/
  case 3:
  case 2:
    hval += asso_values[(unsigned char)str[1]];
  /*FALLTHROUGH*/
  case 1:
    break;
  }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const char *wordlist[] = {"",
                                 "$",
                                 "",
                                 "",
                                 "ipv4",
                                 "[udp]",
                                 "",
                                 "--error",
                                 "udp",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "ipv6",
                                 "",
                                 "",
                                 "$.error",
                                 "",
                                 "",
                                 "[tcp]",
                                 "",
                                 "ipv4.id",
                                 "",
                                 "@udp:port",
                                 "",
                                 "",
                                 "",
                                 "ipv6.hopByHop",
                                 "!out-of-memory",
                                 "ipv6.nextHeader",
                                 "",
                                 "ipv4.protocol.udp",
                                 "ipv4.dst",
                                 "ipv4.flags.reserved",
                                 "",
                                 "",
                                 "ipv6.protocol.udp",
                                 "ipv6.dst",
                                 "@tcp:port",
                                 "",
                                 "",
                                 "udp.dst",
                                 "ipv6.hopLimit",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "tcp",
                                 "",
                                 "ipv4.flags",
                                 "",
                                 "ipv4.version",
                                 "eth.type.ipv4",
                                 "--paylaod",
                                 "",
                                 "",
                                 "ipv6.version",
                                 "eth.type.ipv6",
                                 "!out-of-bounds",
                                 "",
                                 "",
                                 "",
                                 "ipv4.src",
                                 "$.payload",
                                 "",
                                 "",
                                 "",
                                 "ipv6.src",
                                 "",
                                 "",
                                 "--prev",
                                 "ipv4.protocol.tcp",
                                 "ipv4.protocol.icmp",
                                 "",
                                 "_.dst",
                                 "[ipv4]",
                                 "ipv6.protocol.tcp",
                                 "ipv6.protocol.icmp",
                                 "ipv4.fragmentOffset",
                                 "@ipv4:addr",
                                 "[ipv6]",
                                 "tcp.dst",
                                 "eth",
                                 "ipv4.type",
                                 "@ipv6:addr",
                                 "",
                                 "@nested",
                                 "ipv4.ttl",
                                 "tcp.dataOffset",
                                 "",
                                 "",
                                 "ipv6.trafficClass",
                                 "ipv4.protocol",
                                 "[unknown]",
                                 "",
                                 "",
                                 "udp.src",
                                 "ipv6.protocol",
                                 "ipv6.flowLevel",
                                 "",
                                 "",
                                 "tcp.streamId",
                                 "tcp.flags.cwr",
                                 "",
                                 "_.src",
                                 "",
                                 "ipv4.headerLength",
                                 "@int:oct",
                                 "",
                                 "",
                                 "",
                                 "$.index",
                                 "eth.type",
                                 "",
                                 "",
                                 "",
                                 "tcp.seq",
                                 "ipv4.checksum",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "@int:bin",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "ipv4.protocol.igmp",
                                 "tcp.flags",
                                 "",
                                 "",
                                 "tcp.flags.ns",
                                 "ipv6.protocol.igmp",
                                 "",
                                 "[eth]",
                                 "",
                                 "tcp.src",
                                 "@int:dec",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "@novalue",
                                 "",
                                 "",
                                 "--next",
                                 "",
                                 "ipv4.flags.dontFragment",
                                 "$.actualLength",
                                 "",
                                 "",
                                 "",
                                 "@int:hex",
                                 "",
                                 "@date:unix",
                                 "ipv4.totalLength",
                                 "tcp.ack",
                                 "tcp.flags.ack",
                                 "",
                                 "udp.length",
                                 "",
                                 "",
                                 "",
                                 "ipv4.flags.moreFragments",
                                 "",
                                 "",
                                 "udp.checksum",
                                 "tcp.flags.ece",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "--alias",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "@eth:mac",
                                 "",
                                 "",
                                 "$.timestamp",
                                 "",
                                 "",
                                 "!invalid-value",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "@stream",
                                 "ipv6.payloadLength",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "tcp.flags.urg",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "@flags",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "",
                                 "@enum"};

const char *in_word_set(register const char *str, register size_t len) {
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
    register unsigned int key = hash(str, len);

    if (key <= MAX_HASH_VALUE) {
      register const char *s = wordlist[key];

      if (*str == *s && !strcmp(str + 1, s + 1))
        return s;
    }
  }
  return 0;
}
