const { WrappedLayer } = require('./layer')
function findLeaves (root, leaves) {
  if (root === null) {
   return
  }
  if (root.layers.length === 0) {
    leaves.push(root)
  } else {
    for (const layer of root.layers) {
      findLeaves(layer, leaves)
    }
  }
}

const fields = Symbol('fields')
class VirtualFrame {
  constructor (base) {
    this[fields] = {
      base,
      rootLayer: new WrappedLayer(base.rootLayer, this),
    }
  }

  get timestamp () {
    const { base } = this[fields]
    return base.timestamp
  }

  get length () {
    const { base } = this[fields]
    return base.length
  }

  get index () {
    const { base } = this[fields]
    return base.index
  }

  get rootLayer () {
    return this[fields].rootLayer
  }

  get primaryLayer () {
    return this.leafLayers[0] || null
  }

  get leafLayers () {
    const { base } = this[fields]
    const leaves = []
    findLeaves(this.rootLayer, leaves)
    leaves.sort((lhs, rhs) => {
      if (lhs.confidence === rhs.confidence) {
        return lhs.id - rhs.id
      }
      return rhs.confidence - lhs.confidence
    })
    return base.leafLayers
  }

  get sourceId () {
    const { base } = this[fields]
    return base.sourceId
  }

  query (id) {
    for (const leaf of this.leafLayers) {
      for (let parent = leaf; parent !== null; { parent } = parent) {
        if (parent.id === id) {
          return parent
        }
        const layerAttr = parent.attr(id)
        if (layerAttr !== null) {
          return layerAttr
        }
      }
    }
    return null
  }
}

module.exports = VirtualFrame
