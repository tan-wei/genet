#include "token.h"
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace plugkit {

namespace {

extern "C" {
#define register
#include "token_hash.h"
#undef register
}

thread_local std::unordered_map<std::string, Token> localMap;
thread_local std::unordered_map<Token, const char *> localReverseMap;
std::unordered_map<std::string, Token> map;
std::unordered_map<Token, const char *> reverseMap;
std::mutex mutex;
}

Token Token_get(const char *str) {
  if (str == nullptr || str[0] == '\0') {
    return Token_null();
  }

  size_t len = std::strlen(str);
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
    unsigned int key = hash(str, len);
    if (key <= MAX_HASH_VALUE) {
      const char *s = wordlist[key];
      if (strcmp(str, s) == 0) {
        return key + 1;
      }
    }
  }

  // suppress warnings
  if (false) {
    in_word_set(nullptr, 0);
  }

  auto it = localMap.find(str);
  if (it != localMap.end()) {
    return it->second;
  }

  Token id;
  char *data = new char[len + 1]();
  {
    std::lock_guard<std::mutex> lock(mutex);
    const std::string &key = str;
    auto it = map.find(key);
    if (it != map.end()) {
      localMap.insert(*it);
      return it->second;
    }
    id = map.size() + 1 + MAX_HASH_VALUE + 1;
    map.emplace(key, id);
    key.copy(data, len);
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
}
