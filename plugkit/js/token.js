const nativeToken = exports.Token
const tokenMap = new Map()
const tokenReverseMap = new Map()
const joinedMap = new Map()
function tokenGet (str = '') {
  if (typeof str !== 'string') {
    throw new TypeError('First argument must be a string')
  }
  const value = tokenMap.get(str)
  if (value !== undefined) {
    return value
  }
  const id = nativeToken.get(str)
  tokenMap.set(str, id)
  return id
}

function tokenJoin (prefix, str) {
  if (!Number.isInteger(prefix)) {
    throw new TypeError('First argument must be an integer')
  }
  if (typeof str !== 'string') {
    throw new TypeError('Second argument must be a string')
  }
  const key = [prefix, str]
  const value = joinedMap.get(key)
  if (value !== undefined) {
    return value
  }
  const id = nativeToken.join(prefix, str)
  joinedMap.set(key, id)
  return id
}

function tokenString (id) {
  if (!Number.isInteger(id)) {
    throw new TypeError('First argument must be an integer')
  }
  const value = tokenReverseMap.get(id)
  if (value !== undefined) {
    return value
  }
  const str = nativeToken.string(id)
  if (str) {
    tokenReverseMap.set(id, str)
    return str
  }
  return ''
}

function token (strings, ...keys) {
  return tokenGet(String.raw(strings, ...keys))
}

Reflect.defineProperty(token, 'get', { value: tokenGet })
Reflect.defineProperty(token, 'join', { value: tokenJoin })
Reflect.defineProperty(token, 'string', { value: tokenString })
exports.Token = token

class Token {
  // Return a token corresponded with the given string.
  // @return Integer
  get (str = '') {}

  // Return a string corresponded with the given token.
  // @return String
  string (id) {}

  join (prefix, str) {}
}
