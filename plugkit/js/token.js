const nativeToken = exports.Token
const tokenMap = new Map()
const tokenReverseMap = new Map()
const concatMap = new Map()
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

function tokenConcat (prefix, str) {
  let id = 0
  if (Number.isInteger(prefix)) {
    id = prefix
  } else if (typeof prefix === 'string') {
    id = tokenGet(prefix)
  } else {
    throw new TypeError('First argument must be an integer or a string')
  }
  if (typeof str !== 'string') {
    throw new TypeError('Second argument must be a string')
  }
  const key = [id, str]
  const value = concatMap.get(key)
  if (value !== undefined) {
    return value
  }
  const newId = tokenGet(tokenString(id) + str)
  concatMap.set(key, newId)
  return newId
}

function token (strings, ...keys) {
  return tokenGet(String.raw(strings, ...keys))
}

Reflect.defineProperty(token, 'get', { value: tokenGet })
Reflect.defineProperty(token, 'string', { value: tokenString })
Reflect.defineProperty(token, 'concat', { value: tokenConcat })
exports.Token = token

class Token {
  // Return a token corresponded with the given string.
  // @return Integer
  get (str = '') {}

  // Return a string corresponded with the given token.
  // @return String
  string (id) {}
}
