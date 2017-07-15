#ifndef PLUGKIT_MINIID_H
#define PLUGKIT_MINIID_H

#include <cstring>
#include <string>

#define MID_MASK 0x7f7f7f7f7f7f7f7f
#define CMP_TAG(data, bit)                                                  \
  (static_cast<uint64_t>(data & (1 << bit)) << (7 * (bit + 1)))
#define EXP_TAG(data, bit)                                                  \
  (static_cast<uint8_t>(data >> (7 * (bit + 1))) & (1 << bit))

namespace plugkit {

struct miniid {
  uint64_t id;

  constexpr miniid() : id(0) {}
  constexpr miniid(uint64_t num) : id(num) {}
  explicit miniid(const char str[8], uint8_t tag = 0, size_t len = 8);
  std::string str() const;
  uint8_t tag() const;
  bool empty() const;
  bool operator==(miniid other) const;
  bool operator<(miniid other) const;
};

inline miniid::miniid(const char str[8], uint8_t tag, size_t len) : id(0) {
  std::strncpy(reinterpret_cast<char *>(&id), str, len);
  id &= MID_MASK;
  id |= CMP_TAG(tag, 0) | CMP_TAG(tag, 1) | CMP_TAG(tag, 2) |
        CMP_TAG(tag, 3) | CMP_TAG(tag, 4) | CMP_TAG(tag, 5) |
        CMP_TAG(tag, 6) | CMP_TAG(tag, 7);
}

inline std::string miniid::str() const {
  uint64_t masked = id & MID_MASK;
  const char *data = reinterpret_cast<const char *>(&masked);
  if (data[sizeof(data) - 1] == '\0')
    return std::string(data);
  return std::string(data, sizeof(data));
}

inline bool miniid::operator==(miniid other) const { return id == other.id; }

inline bool miniid::operator<(miniid other) const { return id < other.id; }

inline uint8_t miniid::tag() const {
  return EXP_TAG(id, 0) | EXP_TAG(id, 1) | EXP_TAG(id, 2) |
         EXP_TAG(id, 3) | EXP_TAG(id, 4) | EXP_TAG(id, 5) |
         EXP_TAG(id, 6) | EXP_TAG(id, 7);
}

inline bool miniid::empty() const { return id == 0; }

template <size_t len> constexpr uint64_t miniid__(const char *str) {
  return (static_cast<uint64_t>(str[len]) << (8 * len)) | miniid__<len - 1>(str);
}

template <> constexpr uint64_t miniid__<size_t(-1)>(const char *) { return 0; }

template <> constexpr uint64_t miniid__<size_t(-2)>(const char *) { return 0; }

template <size_t len, uint8_t tag> constexpr miniid miniid_(const char *str) {
  return miniid{(miniid__<len>(str) & MID_MASK) | CMP_TAG(tag, 0) |
               CMP_TAG(tag, 1) | CMP_TAG(tag, 2) | CMP_TAG(tag, 3) |
               CMP_TAG(tag, 4) | CMP_TAG(tag, 5) | CMP_TAG(tag, 6) |
               CMP_TAG(tag, 7)};
}
}

#define MID_TAG(str, tag) miniid_<sizeof(str) - 2, tag>(str)
#define MID(str) MID_TAG(str, 0)
#define MID_SUB(str) MID_TAG(str, miniid_sub_layer)

#undef MID_MASK
#undef CMP_TAG
#undef EXP_TAG

#endif
