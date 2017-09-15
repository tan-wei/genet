const kit = require('bindings')('plugkit.node')

const tokenMap = {}
const tokenReverseMap = {}

class Token {
  static get(str) {
    if (str in tokenMap) {
      return tokenMap[str]
    }
    const id = kit.Token.get(str)
    tokenMap[str] = id
    return id
  }

  static string(id) {
    if (id in tokenReverseMap) {
      return tokenReverseMap[id]
    }
    const str = kit.Token.string(id)
    if (str) {
      tokenReverseMap[id] = str
      return str
    }
    return ''
  }
}

module.exports = Token
