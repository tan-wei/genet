const nativeToken = exports.Token
const tokenMap = new Map()
const tokenReverseMap = new Map()

class Token {
  static get(str) {
    const value = tokenMap.get(str)
    if (value !== void 0) {
      return value
    }
    const id = nativeToken.get(str)
    tokenMap.set(str, id)
    return id
  }

  static string(id) {
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
}

exports.Token = Token
