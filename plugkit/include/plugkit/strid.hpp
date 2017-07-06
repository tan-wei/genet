#ifndef PLUGKIT_STRID_H
#define PLUGKIT_STRID_H

#include <cstring>
#include <string>

#define PK_STRID_MASK 0x7f7f7f7f7f7f7f7f
#define PK_CMP_TAG(data, bit)                                                  \
  (static_cast<uint64_t>(data & (1 << bit)) << (7 * (bit + 1)))
#define PK_EXP_TAG(data, bit)                                                  \
  (static_cast<uint8_t>(data >> (7 * (bit + 1))) & (1 << bit))

namespace plugkit {

struct strid {
  uint64_t id;

  constexpr strid() : id(0) {}
  constexpr strid(uint64_t num) : id(num) {}
  explicit strid(const char str[8]);
  std::string str() const;
  uint8_t tag() const;
  bool empty() const;
  bool operator==(strid other) const;
  bool operator<(strid other) const;
};

inline strid::strid(const char str[8]) : id(0) {
  std::strncpy(reinterpret_cast<char *>(&id), str, sizeof(id));
  id &= PK_STRID_MASK;
}

inline std::string strid::str() const {
  uint64_t masked = id & PK_STRID_MASK;
  const char *data = reinterpret_cast<const char *>(&masked);
  if (data[sizeof(data) - 1] == '\0')
    return std::string(data);
  return std::string(data, sizeof(data));
}

inline bool strid::operator==(strid other) const {
  return (id & PK_STRID_MASK) == (other.id & PK_STRID_MASK);
}

inline bool strid::operator<(strid other) const {
  return (id & PK_STRID_MASK) < (other.id & PK_STRID_MASK);
}

inline uint8_t strid::tag() const {
  return PK_EXP_TAG(id, 0) | PK_EXP_TAG(id, 1) | PK_EXP_TAG(id, 2) |
         PK_EXP_TAG(id, 3) | PK_EXP_TAG(id, 4) | PK_EXP_TAG(id, 5) |
         PK_EXP_TAG(id, 6) | PK_EXP_TAG(id, 7);
}

inline bool strid::empty() const { return (id & PK_STRID_MASK) == 0; }

template <size_t len> constexpr uint64_t strid__(const char *str) {
  return (static_cast<uint64_t>(str[len]) << (8 * len)) | strid__<len - 1>(str);
}

template <> constexpr uint64_t strid__<size_t(-1)>(const char *) { return 0; }

template <size_t len, uint8_t tag> constexpr strid strid_(const char *str) {
  return strid{(strid__<len>(str) & PK_STRID_MASK) | PK_CMP_TAG(tag, 0) |
               PK_CMP_TAG(tag, 1) | PK_CMP_TAG(tag, 2) | PK_CMP_TAG(tag, 3) |
               PK_CMP_TAG(tag, 4) | PK_CMP_TAG(tag, 5) | PK_CMP_TAG(tag, 6) |
               PK_CMP_TAG(tag, 7)};
}
}

#define PK_STRID_TAG(str, tag) strid_<sizeof(str) - 2, tag>(str)
#define PK_STRID(str) PK_STRID_TAG(str, 0)
#define PK_STRID_SUB(str) STRID_TAG(str, strid_sub_layer)

#undef PK_STRID_MASK
#undef PK_CMP_TAG
#undef PK_EXP_TAG

#endif
