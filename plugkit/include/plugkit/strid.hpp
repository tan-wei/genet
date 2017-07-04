#ifndef PLUGKIT_STRID_H
#define PLUGKIT_STRID_H

#include <cstring>
#include <string>

struct strid {
  uint64_t id;

  constexpr strid(uint64_t num) : id(num) {}
  explicit strid(const char str[8]);
  std::string str() const;
  bool operator==(strid other);
};

inline strid::strid(const char str[8]) : id(0) {
  std::strncpy(reinterpret_cast<char *>(&id), str, sizeof(id));
}

inline std::string strid::str() const {
  const char *data = reinterpret_cast<const char *>(&id);
  if (data[sizeof(data) - 1] == '\0')
    return std::string(data);
  return std::string(data, sizeof(data));
}

inline bool strid::operator==(strid other) { return this->id == other.id; }

template <size_t len> constexpr strid strid_(const char *str) {
  return strid{((len > 0 ? (uint64_t)str[0] : 0) << (8 * 0)) |
               ((len > 1 ? (uint64_t)str[1] : 0) << (8 * 1)) |
               ((len > 2 ? (uint64_t)str[2] : 0) << (8 * 2)) |
               ((len > 3 ? (uint64_t)str[3] : 0) << (8 * 3)) |
               ((len > 4 ? (uint64_t)str[4] : 0) << (8 * 4)) |
               ((len > 5 ? (uint64_t)str[5] : 0) << (8 * 5)) |
               ((len > 6 ? (uint64_t)str[6] : 0) << (8 * 6)) |
               ((len > 7 ? (uint64_t)str[7] : 0) << (8 * 7))};
}

#define PK_STRID(str) strid_<sizeof(str) - 1>(str)

#endif
