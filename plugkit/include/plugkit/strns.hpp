#ifndef PLUGKIT_STRNS_H
#define PLUGKIT_STRNS_H

#include "strid.hpp"
#include <array>

namespace plugkit {

struct strns {
  enum flag { strns_sub_layer = 1 };

  std::array<strid, 3> id;

  constexpr strns() : id() {}
  constexpr strns(strid id1, strid id2, strid id3) : id{{id1, id2, id3}} {}
  constexpr strns(strid id1, strid id2) : id{{strid(), id1, id2}} {}
  constexpr strns(strid id1) : id{{strid(), strid(), id1}} {}
  std::string str() const;
  bool match(const strns &other) const;
};

inline std::string strns::str() const {
  return (id[0].empty() ? "" : id[0].str() + " ") +
         (id[1].empty() ? "" : id[1].str() + " ") +
         (id[2].empty() ? "" : id[2].str());
}

inline bool strns::match(const strns &other) const {
  return (other.id[2].empty() || other.id[2] == id[2]) &&
         (other.id[1].empty() || other.id[1] == id[1]) &&
         (other.id[0].empty() || other.id[0] == id[0]);
}
}

#endif
