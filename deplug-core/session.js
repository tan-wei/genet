const dissectors = []
export default class Session {
  static registerDissector (file) {
    dissectors.push(require(file).factory)
  }

  static get dissectors () {
    return dissectors
  }
}
