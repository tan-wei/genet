#ifndef PLUGKIT_STRNS_H
#define PLUGKIT_STRNS_H

#include "strid.hpp"
#include <array>
#include <functional>

namespace plugkit {

struct strns {
  enum flag { strns_sub_layer = 1 };

  std::array<strid, 3> id;

  constexpr strns() : id{{strid(), strid(), strid()}} {}
  constexpr strns(strid id1, strid id2, strid id3) : id{{id1, id2, id3}} {}
  constexpr strns(strid id1, strid id2) : id{{strid(), id1, id2}} {}
  constexpr strns(strid id1) : id{{strid(), strid(), id1}} {}
  strns(const char *str);

  std::string str() const;
  strid primary() const;
  bool empty() const;
  bool match(const strns &other) const;
  bool operator==(const strns &other) const;
};

inline strns::strns(const char *str) {
  size_t len = std::strlen(str);
  size_t index = id.size();
  size_t idlen = 0;
  for (int cursor = len - 1; cursor > -2 && index > 0; --cursor) {
    ++idlen;
    if (cursor < 0 || str[cursor] == ' ') {
      if (str[cursor + 1] == '*') {
        id[index - 1] = strid(str + cursor + 2, 1, idlen - 2);
      } else {
        id[index - 1] = strid(str + cursor + 1, 0, idlen - 1);
      }
      --index;
      idlen = 0;
    }
  }
}

inline std::string strns::str() const {
  std::string buf;
  for (size_t i = 0; i < id.size(); ++i) {
    if (!id[i].empty()) {
      if (id[i].tag())
        buf += "*";
      buf += id[i].str();
      if (i < id.size() - 1)
        buf += " ";
    }
  }
  return buf;
}

inline strid strns::primary() const {
  for (int i = id.size() - 1; i >= 0; --i) {
    if (!id[i].tag())
      return id[i];
  }
  return strid();
}

inline bool strns::empty() const {
  for (strid n : id) {
    if (!n.empty())
      return false;
  }
  return true;
}

inline bool strns::match(const strns &other) const {
  for (int i = id.size() - 1; i >= 0; --i) {
    if (!other.id[i].empty() && other.id[i].id != id[i].id)
      return false;
  }
  return true;
}

inline bool strns::operator==(const strns &other) const {
  for (size_t i = 0; i < id.size(); ++i) {
    if (id[i].id != other.id[i].id)
      return false;
  }
  return true;
}

template <size_t len> constexpr strid strid_ns_(const char *str) {
  return (str[0] == '*' ? strid_<len - 1, 1>(str + 1) : strid_<len, 0>(str));
}

#define PK_STRNS(id) strid_ns_<sizeof(id) - 2>(id)
}

namespace std {
template <> struct hash<plugkit::strns> {
  size_t operator()(const plugkit::strns &ns) const {
    size_t h = 0;
    for (plugkit::strid id : ns.id) {
      h ^= hash<uint64_t>()(id.id);
    }
    return h;
  }
};
}

#endif
