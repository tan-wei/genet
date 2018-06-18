const native = require('./binding')
const { EventEmitter } = require('events')

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

  pushFrames (frames, link) {
    this._sess.pushFrames(frames, link)
  }

  frames (start, end) {
    return this._sess.frames(start, end)
  }

  filteredFrames (id, start, end) {
    return this._sess.filteredFrames(id, start, end)
  }

  setFilter (id, filter = '') {
    this._sess.setFilter(SymbolPool.get(id), `
        ((frame) => frame.index % 2)
        `)
  }

  readFile (name, format = '') {
    return this._sess.readFile(name, format)
  }

  writeFile (name, format = '', filter = '') {
    return this._sess.writeFile(name, format, filter)
  }

  startPcap () {
    this._sess.startPcap()
    this.emit('event', {
      type: 'pcap',
      running: true,
    })
  }

  stopPcap () {
    this._sess.stopPcap()
    this.emit('event', {
      type: 'pcap',
      running: false,
    })
  }

  get context () {
    return this._sess.context
  }

  get length () {
    return this._sess.length
  }
}

Reflect.defineProperty(Session, 'Profile', { value: native.SessionProfile })

module.exports = Session
