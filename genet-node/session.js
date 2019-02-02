const native = require('./binding')
const { Token, Bytes } = native
const { Disposable } = require('disposables')
const { EventEmitter } = require('events')
function treefy (layerStack) {
  if (layerStack.length === 0) {
    return []
  }
  const layers = [layerStack[0]]
  let [cursor] = layerStack
  cursor.children = cursor.children || []
  layerStack.shift()
  for (const layer of layerStack) {
    layer.children = layer.children || []

    let cursorAddr = Bytes.address(cursor.data)
    const layerAddr = Bytes.address(layer.data)
    while (layerAddr < cursorAddr) {
      cursor = cursor.parent
      cursorAddr = Bytes.address(cursor.data)
    }
    cursor.children.push(layer)
    layer.parent = cursor
    cursor = layer
  }
  return layers
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
      [this._root] = treefy(this._frame.layers)
    }
    return this._root
  }

  get primary () {
    if (!this.root) {
      return null
    }
    function fistChild (layer) {
      if (layer.children.length === 0) {
        return layer
      }
      return fistChild(layer.children[0])
    }
    return fistChild(this.root)
  }

  query (id) {
    return this._frame.query(id)
  }
}

class Session extends EventEmitter {
  constructor (profile, options) {
    super()
    this._options = options
    this._sess = new native.Session(profile, (json) => {
      const event = JSON.parse(json)
      switch (event.type) {
        case 'frames':
          this._status.frames = event.length
          break
        case 'async_frames':
          this._status.asyncFrames = event.length
          break
        case 'filtered_frames':
          this._status.filters[event.id] =
            { frames: event.length }
          break
        case 'error':
          this.emit('error', event.error)
          break
        default:
      }
      this.emit('update', event)
    })
    this._streams = []
    this._streamReaders = new Set()
    this._status = {
      filters: {},
      frames: 0,
      asyncFrames: 0,
      stream: false,
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
    return this._sess.filteredFrames(id, start, end)
  }

  get status () {
    return this._status
  }

  setFilter (id, filter = '') {
    this._sess.setFilter(id, filter)
    if (filter === '') {
      Reflect.deleteProperty(this._status.filters, id)
    }
  }

  createReader (id, url) {
    const handle = this._sess.createReader(id, url.toString())
    if (handle === 0) {
      throw new Error(`failed to invoke reader: ${id}`)
    }
    const disposable = new Disposable(() => {
      this._sess.closeReader(handle)
    })
    disposable.promise = new Promise((res, rej) => {
      this.on('update', (event) => {
        if (event.id === handle && event.type === 'output') {
          if (event.error === null) {
            res()
          } else {
            rej(new Error(event.error))
          }
        }
      })
    })
    return disposable
  }

  async createWriter (id, url, filter = '') {
    const handle = this._sess.createWriter(
      id, url.toString(), filter)
    if (handle === 0) {
      throw new Error(`failed to invoke writer: ${id}`)
    }
    const disposable = new Disposable(() => {
      this._sess.closeReader(handle)
    })
    disposable.promise = new Promise((res, rej) => {
      this.on('update', (event) => {
        if (event.id === handle && event.type === 'input') {
          if (event.error === null) {
            res()
          } else {
            rej(new Error(event.error))
          }
        }
      })
    })
    return disposable
  }

  regiterStreamReader (id, url) {
    const reader = {
      id,
      url,
    }
    this._streamReaders.add(reader)
    return new Disposable(() => {
      this._streamReaders.delete(reader)
    })
  }

  startStream () {
    this.stopStream()
    this._streams = Array.from(this._streamReaders)
      .map(({ id, url }) => this.createReader(id, url.toString()))
    this._status.stream = true
  }

  stopStream () {
    for (const handle of this._streams) {
      handle.dispose()
    }
    this._streams = []
    this._status.stream = false
  }

  get length () {
    return this._sess.length
  }

  get profile () {
    return JSON.parse(this._sess.profile)
  }
}

class Profile extends native.Session.Profile { }

Reflect.defineProperty(Session, 'Profile', { value: Profile })

module.exports = Session
