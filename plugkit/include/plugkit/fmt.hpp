#ifndef PLUGKIT_FMT_H
#define PLUGKIT_FMT_H

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace plugkit {
namespace fmt {

template <class S> class Reader {
public:
  Reader(const S &slice);
  S slice(size_t length);
  S left() const;
  template <class T> T readLE();
  template <class T> T readBE();
  std::pair<uint32_t, uint32_t> lastRange() const;
  std::string lastError() const;

private:
  S slice_;
  size_t offset_;
  std::pair<uint32_t, uint32_t> lastRange_;
  std::string lastError_;
};

template <class S>
Reader<S>::Reader(const S &slice)
    : slice_(slice), offset_(0), lastRange_(), lastError_() {}

template <class S> S Reader<S>::slice(size_t length) {
  if (!lastError_.empty())
    return S();
  if (offset_ + length > slice_.size()) {
    lastRange_.first = 0;
    lastRange_.second = 0;
    lastError_ = "unexpected EOS";
    return S();
  }
  const S &s = slice_.slice(offset_, length);
  lastRange_.first = offset_;
  lastRange_.second = offset_ + length;
  offset_ += length;
  return s;
}

template <class S> S Reader<S>::left() const {
  return slice_.slice(offset_, slice_.size() - offset_);
}

template <class S> template <class T> T Reader<S>::readLE() {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  if (!lastError_.empty())
    return T();
  if (offset_ + sizeof(T) > slice_.size()) {
    lastRange_.first = 0;
    lastRange_.second = 0;
    lastError_ = "unexpected EOS";
    return T();
  }
  const T &value = *reinterpret_cast<const T *>(slice_.data() + offset_);
  lastRange_.first = offset_;
  lastRange_.second = offset_ + sizeof(T);
  offset_ += sizeof(T);
  return value;
}

template <class S> template <class T> T Reader<S>::readBE() {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  if (!lastError_.empty())
    return T();
  if (offset_ + sizeof(T) > slice_.size()) {
    lastRange_.first = 0;
    lastRange_.second = 0;
    lastError_ = "unexpected EOS";
    return T();
  }
  char data[sizeof(T)];
  const char *begin = slice_.data() + offset_;
  std::reverse_copy(begin, begin + sizeof(T), data);
  const char *alias = data;
  const T &value = *reinterpret_cast<const T *>(alias);
  lastRange_.first = offset_;
  lastRange_.second = offset_ + sizeof(T);
  offset_ += sizeof(T);
  return value;
}

template <class S> std::pair<uint32_t, uint32_t> Reader<S>::lastRange() const {
  return lastRange_;
}

template <class S> std::string Reader<S>::lastError() const {
  return lastError_;
}

template <class T, class S> T readLE(const S &slice, size_t offset = 0) {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  return *reinterpret_cast<const T *>(slice.data() + offset);
}

template <class T, class S> T readBE(const S &slice, size_t offset = 0) {
  static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
  char data[sizeof(T)];
  const char *begin = slice.data() + offset;
  std::reverse_copy(begin, begin + sizeof(T), data);
  const char *alias = data;
  return *reinterpret_cast<const T *>(alias);
}

template <class S>
std::string toHex(const S &slice, int group = 0, int width = 2,
                  char sep = ':') {
  std::stringstream stream;
  stream << std::hex << std::setfill('0');
  for (size_t i = 0; i < slice.size(); ++i) {
    stream << std::setw(width) << +static_cast<uint8_t>(slice[i]);
    if (group > 0 && (i + 1) % group == 0 && i < slice.size() - 1) {
      stream << sep;
    }
  }
  return stream.str();
}

template <class S>
std::string toDec(const S &slice, int group = 0, int width = 0,
                  char sep = '.') {
  std::stringstream stream;
  stream << std::dec << std::setfill('0');
  for (size_t i = 0; i < slice.size(); ++i) {
    stream << std::setw(width) << +static_cast<uint8_t>(slice[i]);
    if (group > 0 && (i + 1) % group == 0 && i < slice.size() - 1) {
      stream << sep;
    }
  }
  return stream.str();
}

template <class S> std::string range(const S &base, const S &slice) {
  const char *baseBegin = base.data();
  const char *baseEnd = baseBegin + base.size();
  const char *sliceBegin = slice.data();
  const char *sliceEnd = sliceBegin + slice.size();
  if (baseBegin <= sliceBegin && sliceBegin < baseEnd && baseBegin < sliceEnd &&
      sliceEnd <= baseEnd) {
    std::string begin;
    std::string end;
    if (baseBegin != sliceBegin) {
      begin = std::to_string(sliceBegin - baseBegin);
    }
    if (baseEnd != sliceEnd) {
      end = std::to_string(sliceBegin - baseBegin + slice.size());
    }
    return begin + ":" + end;
  }
  return std::string();
}

template <class M, class K = typename M::key_type,
          class V = typename M::mapped_type>
std::string enums(const M &table, const K &value, const V &defval = V()) {
  const auto &it = table.find(value);
  if (it != table.end()) {
    return it->second;
  }
  return defval;
}
}
}

#endif
