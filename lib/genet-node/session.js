const native = require('./binding')
const { Disposable } = require('disposables')
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

  createReader (id, arg = '') {
    const handle = this._sess.createReader(id, arg)
    return new Disposable(() => {
      this._sess.closeReader(handle)
    })
  }

  createWriter (id, arg = '') {
    const handle = this._sess.createWriter(id, arg)
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
