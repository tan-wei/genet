#include "token.h"
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace plugkit {

namespace {
#define register
#include "token_hash.h"
#undef register

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
  if (false) {
    in_word_set(nullptr, 0);
  }

  {
    const std::string &key = str;
    auto it = localMap.find(key);
    if (it != localMap.end()) {
      return it->second;
    }
  }

  std::lock_guard<std::mutex> lock(mutex);
  const std::string &key = str;
  auto it = map.find(key);
  if (it != map.end()) {
    localMap.insert(*it);
    return it->second;
  }
  Token id = map.size() + 1 + MAX_HASH_VALUE + 1;
  map.insert(std::make_pair(key, id));
  char *data = new char[key.size() + 1]();
  key.copy(data, key.size());
  reverseMap.insert(std::make_pair(id, data));
  return id;
}

Token Token_null() { return Token{0}; }

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
