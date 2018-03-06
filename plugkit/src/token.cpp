#include "token.hpp"
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace std {
template <>
struct hash<std::pair<plugkit::Token, std::string>> {
  inline size_t
  operator()(const pair<plugkit::Token, std::string> &v) const {
    return v.first ^ std::hash<std::string>()(v.second);
  }
};
} // namespace std

namespace plugkit {

namespace {

#define register
#include "token_hash.h"
#undef register

thread_local std::unordered_map<std::string, Token> localMap;
thread_local std::unordered_map<Token, const char *> localReverseMap;
thread_local std::unordered_map<std::pair<Token, std::string>, Token> pairMap;
std::unordered_map<std::string, Token> map;
std::unordered_map<Token, const char *> reverseMap;
std::mutex mutex;
} // namespace

Token Token_literal_(const char *str, size_t length) {
  if (str == nullptr || str[0] == '\0') {
    return Token_null();
  }

  if (length <= MAX_WORD_LENGTH && length >= MIN_WORD_LENGTH) {
    unsigned int key = hash(str, length);
    if (key <= MAX_HASH_VALUE) {
      const char *s = wordlist[key];
      if (strncmp(str, s, length) == 0) {
        return key + 1;
      }
    }
  }

  // suppress warnings
  if (false) {
    in_word_set(nullptr, 0);
  }

  const std::string key(str, length);
  auto it = localMap.find(key);
  if (it != localMap.end()) {
    return it->second;
  }

  Token id;
  char *data = new char[length + 1]();
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = map.find(key);
    if (it != map.end()) {
      localMap.insert(*it);
      return it->second;
    }
    id = map.size() + 1 + MAX_HASH_VALUE + 1;
    map.emplace(key, id);
    key.copy(data, length);
    reverseMap.emplace(id, data);
  }
  localMap.emplace(data, id);
  localReverseMap.emplace(id, data);
  return id;
}

const char *Token_string(Token token) {
  if (token == Token_null()) {
    return "";
  }
  if (token <= MAX_HASH_VALUE + 1) {
    return wordlist[token - 1];
  }
  {
    auto it = localReverseMap.find(token);
    if (it != localReverseMap.end()) {
      return it->second;
    }
  }
  std::lock_guard<std::mutex> lock(mutex);
  auto it = reverseMap.find(token);
  if (it != reverseMap.end()) {
    localReverseMap.insert(*it);
    return it->second;
  }
  return "";
}

Token Token_concat(Token prefix, const char *str, size_t length)
{
  const auto &pair = std::make_pair(prefix, std::string(str, length));
  const auto it = pairMap.find(pair);
  if (it != pairMap.end()) {
    return it->second;
  }
  const auto &key = Token_string(prefix) + pair.second;
  Token token = Token_literal_(key.c_str(), key.size());
  pairMap[pair] = token;
  return token;
}

} // namespace plugkit
