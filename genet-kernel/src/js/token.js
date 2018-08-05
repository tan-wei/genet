const nativeToken = exports.Token
const tokenMap = new Map()
const tokenReverseMap = new Map()
function tokenGet (str = '') {
  if (typeof str !== 'string') {
    throw new TypeError('First argument must be a string')
  }
  if (str.length === 0) {
    return 0
  }
  const value = tokenMap.get(str)
  if (value !== undefined) {
    return value
  }
  const id = nativeToken.get(str)
  tokenMap.set(str, id)
  return id
}

function tokenString (id) {
  if (!Number.isInteger(id)) {
    throw new TypeError('First argument must be an integer')
  }
  if (id === 0) {
    return ''
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
Reflect.defineProperty(token, 'string', { value: tokenString })
exports.Token = token
