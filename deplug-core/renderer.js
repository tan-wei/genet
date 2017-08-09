const properties = {}
const layers = {}
export default class Renderer {
  static registerProperty (type, renderer) {
    properties[type] = renderer
  }

  static registerLayer (id, renderer) {
    layers[id] = renderer
  }

  static forProperty (type) {
    if (type in properties) {
      return properties[type]
    }
    return properties['']
  }

  static forLayer (id) {
    if (id in layers) {
      return layers[id]
    }
    return layers['']
  }
}
