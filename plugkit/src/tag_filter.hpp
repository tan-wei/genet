#ifndef PLUGKIT_TAG_FILTER_HPP
#define PLUGKIT_TAG_FILTER_HPP

#include <vector>
#include "token.h"

namespace plugkit {

class TagFilter {
public:
  TagFilter();
  TagFilter(const std::vector<Token> &tags);
  bool match(const std::vector<Token> &tags) const;

private:
  uint64_t hash = 0;
  std::vector<Token> filters;
};
}

#endif
