const dissectors = []
const linkLayers = []
export default class Session {
  static registerDissector (file) {
    dissectors.push(require(file).factory)
  }

  static registerLinkLayer (layer) {
    linkLayers.push(layer)
  }

  static get dissectors () {
    return dissectors
  }

  static get linkLayers () {
    return linkLayers
  }
}
