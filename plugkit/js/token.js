const nativeToken = exports.Token
const tokenMap = {}
const tokenReverseMap = {}

class Token {
  static get(str) {
    if (str in tokenMap) {
      return tokenMap[str]
    }
    const id = nativeToken.get(str)
    tokenMap[str] = id
    return id
  }

  static string(id) {
    if (id in tokenReverseMap) {
      return tokenReverseMap[id]
    }
    const str = nativeToken.string(id)
    if (str) {
      tokenReverseMap[id] = str
      return str
    }
    return ''
  }
}

exports.Token = Token
