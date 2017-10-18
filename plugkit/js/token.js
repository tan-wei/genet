const nativeToken = exports.Token
const tokenMap = new Map()
const tokenReverseMap = new Map()

function token(strings, ...keys) {
  return token_get(String.raw(strings, ...keys))
}

function token_get(str = '') {
  if (typeof str !== 'string') {
    throw new TypeError('First argument must be a string')
  }
  const value = tokenMap.get(str)
  if (value !== void 0) {
    return value
  }
  const id = nativeToken.get(str)
  tokenMap.set(str, id)
  return id
}

function token_string(id) {
  if (!Number.isInteger(id)) {
    throw new TypeError('First argument must be an integer')
  }
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
  // Return a token corresponded with the given string.
  // @return Integer
  get(str = '') {}

  // Return a string corresponded with the given token.
  // @return String
  string(id) {}
}
