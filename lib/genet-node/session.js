const native = require('./binding')
const { Disposable } = require('disposables')
const { EventEmitter } = require('events')
const FilterCompiler = require('./filter')

const pool = new Map()
class SymbolPool {
  static get (str) {
    let id = pool.get(str)
    if (typeof id === 'undefined') {
      id = pool.size
      pool.set(str, id)
    }
    return id
  }
}

function treefy (layerStack, indexStack) {
  function consume (n, layerStack, indexStack) {
    const indices = indexStack.splice(0, n)
    const layers = layerStack.splice(0, n)
    for (let i = 0; i < indices.length; ++i) {
      layers[i].children = (layers[i].children || [])
        .concat(consume(indices[i], layerStack, indexStack))
    }
    return layers
  }

  const layers = [].concat(layerStack)
  const indices = [].concat(indexStack)
  const root = consume(1, layers, indices)
  const len = layerStack.length - layers.length
  if (indices.length >= len) {
    consume(len, [].concat(layerStack), indices)
  }
  return root
}

class Frame {
  constructor (frame) {
    this._frame = frame
    this._root = null
  }

  get index () {
    return this._frame.index
  }

  get root () {
    if (!this._root) {
      this._root = treefy(this._frame.layers, this._frame.treeIndices)

    }
    return this._root
  }
}

class Session extends EventEmitter {
  constructor (profile) {
    super()
    this._sess = new native.Session(profile)
    this._sess.callback = (event) => {
      this.emit('event', event)
    }
  }

  close () {
    this._sess.close()
  }

  frames (start, end) {
    return this._sess
      .frames(start, end)
      .map((frame) => new Frame(frame))
  }

  filteredFrames (id, start, end) {
    return this._sess
      .filteredFrames(id, start, end)
      .map((frame) => new Frame(frame))
  }

  setFilter (id, filter = '') {
    const filterCompiler = new FilterCompiler()
    const body = filterCompiler.compile(filter, { built: false }).linked
    this._sess.setFilter(SymbolPool.get(id), body)
  }

  createReader (id, arg = '') {
    const handle = this._sess.createReader(id, arg)
    if (handle === 0) {
      throw new Error(`unregistered ID: ${id}`)
    }
    return new Disposable(() => {
      this._sess.closeReader(handle)
    })
  }

  createWriter (id, arg = '') {
    const handle = this._sess.createWriter(id, arg)
    if (handle === 0) {
      throw new Error(`unregistered ID: ${id}`)
    }
    return new Disposable(() => {
      this._sess.closeWriter(handle)
    })
  }

  get length () {
    return this._sess.length
  }
}

class Profile extends native.SessionProfile {}

Reflect.defineProperty(Session, 'Profile', { value: Profile })

module.exports = Session
