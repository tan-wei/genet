const dissectors = []
const streamDissectors = []
const linkLayers = []
export default class Session {
  static registerDissector (file) {
    dissectors.push(require(file).factory)
  }

  static registerStreamDissector (file) {
    streamDissectors.push(require(file).factory)
  }

  static registerLinkLayer (layer) {
    linkLayers.push(layer)
  }

  static get dissectors () {
    return dissectors
  }

  static get streamDissectors () {
    return streamDissectors
  }

  static get linkLayers () {
    return linkLayers
  }
}
