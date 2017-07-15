#ifndef PLUGKIT_MININS_H
#define PLUGKIT_MININS_H

#include "miniid.hpp"
#include <array>
#include <functional>

namespace plugkit {

struct minins {
  enum flag { minins_sub_layer = 1 };

  std::array<miniid, 3> id;

  constexpr minins() : id{{miniid(), miniid(), miniid()}} {}
  constexpr minins(miniid id1, miniid id2, miniid id3) : id{{id1, id2, id3}} {}
  constexpr minins(miniid id1, miniid id2) : id{{miniid(), id1, id2}} {}
  constexpr minins(miniid id1) : id{{miniid(), miniid(), id1}} {}
  minins(const char *str);

  std::string str() const;
  miniid primary() const;
  bool empty() const;
  bool match(const minins &other) const;
  bool operator==(const minins &other) const;
  bool operator!=(const minins &other) const;
};

inline minins::minins(const char *str) {
  size_t len = std::strlen(str);
  size_t index = id.size();
  size_t idlen = 0;
  for (int cursor = len - 1; cursor > -2 && index > 0; --cursor) {
    ++idlen;
    if (cursor < 0 || str[cursor] == ' ') {
      if (str[cursor + 1] == '*') {
        id[index - 1] = miniid(str + cursor + 2, 1, idlen - 2);
      } else {
        id[index - 1] = miniid(str + cursor + 1, 0, idlen - 1);
      }
      --index;
      idlen = 0;
    }
  }
}

inline std::string minins::str() const {
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

inline miniid minins::primary() const {
  for (int i = id.size() - 1; i >= 0; --i) {
    if (!id[i].tag())
      return id[i];
  }
  return miniid();
}

inline bool minins::empty() const {
  for (miniid n : id) {
    if (!n.empty())
      return false;
  }
  return true;
}

inline bool minins::match(const minins &other) const {
  for (int i = id.size() - 1; i >= 0; --i) {
    if (!other.id[i].empty() && other.id[i].id != id[i].id)
      return false;
  }
  return true;
}

inline bool minins::operator==(const minins &other) const {
  for (size_t i = 0; i < id.size(); ++i) {
    if (id[i].id != other.id[i].id)
      return false;
  }
  return true;
}

inline bool minins::operator!=(const minins &other) const {
  for (size_t i = 0; i < id.size(); ++i) {
    if (id[i].id != other.id[i].id)
      return true;
  }
  return false;
}

template <size_t len> constexpr miniid miniid_ns_(const char *str) {
  return (str[0] == '*' ? miniid_<len - 1, 1>(str + 1) : miniid_<len, 0>(str));
}

#define MNS(id) miniid_ns_<sizeof(id) - 2>(id)
}

namespace std {
template <> struct hash<plugkit::minins> {
  size_t operator()(const plugkit::minins &ns) const {
    size_t h = 0;
    for (plugkit::miniid id : ns.id) {
      h ^= hash<uint64_t>()(id.id);
    }
    return h;
  }
};
}

#endif
