#include "token.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace plugkit {

namespace {

std::unordered_map<std::string, Token> map;
std::unordered_map<Token, const char *> reverseMap;
std::mutex mutex;
}

Token Token_get(const char *str) {
  if (str == nullptr || str[0] == '\0') {
    return Token_null();
  }
  std::lock_guard<std::mutex> lock(mutex);
  const std::string &key = str;
  auto it = map.find(key);
  if (it != map.end()) {
    return it->second;
  }
  Token id = map.size() + 1;
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
  std::lock_guard<std::mutex> lock(mutex);
  auto it = reverseMap.find(token);
  if (it != reverseMap.end()) {
    return it->second;
  }
  return "";
}
}
