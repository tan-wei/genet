#include "tag_filter.hpp"

namespace plugkit {

TagFilter::TagFilter() {}

TagFilter::TagFilter(const std::vector<Token> &tags) : filters(tags) {
  for (Token tag : filters) {
    hash |= (0x1 << (tag % 64));
  }
}

bool TagFilter::match(const std::vector<Token> &tags) const {
  uint64_t bloom = 0;
  for (Token tag : tags) {
    bloom |= (0x1 << (tag % 64));
  }
  if ((hash & bloom) != hash) {
    return false;
  }
  for (Token filter : filters) {
    bool found = false;
    for (Token tag : tags) {
      if (filter == tag) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
}
}
