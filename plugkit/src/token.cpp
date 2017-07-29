#include "token.h"
#include <string>
#include <unordered_map>
#include <mutex>

namespace {

std::unordered_map<std::string, Token> map = {{"", 0}};
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
  Token id = map.size();
  map.insert(std::make_pair(key, id));
  return id;
}

Token Token_null() { return Token{0}; }
