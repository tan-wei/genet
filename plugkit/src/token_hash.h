/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gperf -LANSI-C -G --output-file=plugkit/src/token_hash.h plugkit/src/token.keys  */
/* Computed positions: -k'2,4-5,16,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif


#define TOTAL_KEYWORDS 82
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 24
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 290
/* maximum key range = 287, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register unsigned int len)
{
  static unsigned short asso_values[] =
    {
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291,   0,  10, 291, 291, 291,
      291, 291,   0, 291,   5, 291, 291, 291,  85, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      115, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291,   0, 291, 291, 291,  50, 291,  45,
        5,  65,  10,  70,   5,   0, 291,  90,  65, 110,
       30,   0,   0,  20,  40,  30,  15,   0,   5, 291,
       20,   0, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291, 291, 291, 291, 291,
      291, 291, 291, 291, 291, 291
    };
  register unsigned int hval = len;

  switch (hval)
    {
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
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const char * wordlist[] =
  {
    "", "", "", "",
    "ipv4",
    "[udp]",
    "", "",
    "udp",
    "", "",
    "[ipv4]",
    "", "",
    "ipv6",
    "",
    "[ipv6]",
    "", "", "",
    "[tcp]",
    "",
    "ipv4.id",
    "eth",
    "", "", "", "",
    "ipv6.hopByHop",
    "!out-of-memory",
    "", "",
    "ipv4.protocol.udp",
    "ipv4.dst",
    "", "",
    "ipv4.totalLength",
    "ipv6.protocol.udp",
    "ipv6.dst",
    ".dst",
    "", "",
    "udp.dst",
    "ipv6.hopLimit",
    "", "", "",
    "ipv4.headerLength",
    "tcp",
    "",
    "ipv4.flags",
    "",
    "ipv4.version",
    "eth.type.ipv4",
    "ipv4.fragmentOffset",
    "@ipv4:addr",
    "",
    "ipv6.version",
    "eth.type.ipv6",
    "!out-of-bounds",
    "@ipv6:addr",
    "", "",
    "ipv4.src",
    "", "", "", "",
    "ipv6.src",
    "",
    "ipv6.nextHeader",
    "",
    "ipv4.protocol.tcp",
    "ipv4.protocol.icmp",
    "ipv4.flags.reserved",
    "[eth]",
    "",
    "ipv6.protocol.tcp",
    "ipv6.protocol.icmp",
    "", "", "",
    "tcp.dst",
    "ipv4.ttl",
    "ipv4.type",
    "", "",
    "@nested",
    "ipv4.protocol",
    "tcp.dataOffset",
    "", "",
    "ipv6.trafficClass",
    "ipv6.protocol",
    "ipv6.flowLevel",
    "udp.length",
    "",
    "udp.src",
    "ipv4.protocol.igmp",
    "", "", "",
    "tcp.streamId",
    "ipv6.protocol.igmp",
    "tcp.flags",
    "", "",
    "tcp.flags.ns",
    "ipv6.payloadLength",
    "", "", "",
    "tcp.seq",
    "eth.type",
    "",
    "@date:unix",
    "", "",
    "tcp.flags.cwr",
    "", "", "", "",
    "@int:oct",
    ".src",
    "", "", "",
    "@int:hex",
    "[unknown]",
    "", "", "",
    "ipv4.checksum",
    "!invalid-value",
    "", "",
    "tcp.src",
    "@int:bin",
    "", "", "", "",
    "tcp.flags.ece",
    "", "", "", "",
    "tcp.flags.urg",
    "", "", "", "",
    "@int:dec",
    "", "", "", "",
    "@novalue",
    "", "", "", "",
    "ipv4.flags.dontFragment",
    "", "",
    "@flags",
    "",
    "tcp.flags.ack",
    "", "", "", "", "", "", "", "",
    ".length",
    "",
    "ipv4.flags.moreFragments",
    "", "",
    "udp.checksum",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "",
    "tcp.ack",
    "", "", "", "", "",
    "@eth:mac",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "",
    "@stream",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",
    "",
    "@enum"
  };

const char *
in_word_set (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key];

          if (*str == *s && !strcmp (str + 1, s + 1))
            return s;
        }
    }
  return 0;
}
