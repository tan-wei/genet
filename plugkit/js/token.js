const nativeToken = exports.Token
const tokenMap = new Map()
const tokenReverseMap = new Map()

function token(strings, ...keys) {
  return token_get(String.raw(strings, ...keys))
}

function token_get(str) {
  const value = tokenMap.get(str)
  if (value !== void 0) {
    return value
  }
  const id = nativeToken.get(str)
  tokenMap.set(str, id)
  return id
}

function token_string(id) {
  const value = tokenReverseMap.get(id)
  if (value !== void 0) {
    return value
  }
  const str = nativeToken.string(id)
  if (str) {
    tokenReverseMap.set(id, str)
    return str
  }
  return ''
}

Object.defineProperty(token, 'get',    { value: token_get    })
Object.defineProperty(token, 'string', { value: token_string })
exports.Token = token

class Token {
  // @return Integer
  get(str) {}

  // @return String
  string(id) {}
}
