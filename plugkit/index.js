const kit = require('./plugkit')
const fs = require('fs')
const { promisify } = require('util')
const EventEmitter = require('events')
const FilterCompiler = require('./filter')
const InspectorServer = require('./inspector')

const fields = Symbol('fields')
const promiseReadFile = promisify(fs.readFile)
class Session extends EventEmitter {
  constructor (sess, options) {
    super()
    this[fields] = Object.assign({ sess }, options)

    if (options.enableDebugSession) {
      this[fields].inspector = new InspectorServer(sess)
    }

    this.status = { capture: false }
    this.filter = {}
    this.frame = { frames: 0 }

    sess.setStatusCallback((status) => {
      this.status = status
      this.emit('status', status)
    })
    sess.setFilterCallback((filter) => {
      this.filter = filter
      this.emit('filter', filter)
    })
    sess.setFrameCallback((frame) => {
      this.frame = frame
      this.emit('frame', frame)
    })
    sess.setLoggerCallback((log) => {
      this.emit('log', log)
    })
    sess.setEventCallback((type, data) => {
      this.emit(`event:${type}`, JSON.parse(data))
    })
  }

  importFile (file) {
    return new Promise((res) => {
      const id = this[fields].sess.importFile(file)
      const listener = (data) => {
        if (data.id === id && data.progress >= 1) {
          this.removeListener('event:importer', listener)
          res()
        }
      }
      this.on('event:importer', listener)
    })
  }

  exportFile (file, filter = '') {
    const { filterCompiler } = this[fields]
    const body = filterCompiler.compile(filter, { built: false }).linked
    return new Promise((res) => {
      const id = this[fields].sess.exportFile(file, body)
      const listener = (data) => {
        if (data.id === id && data.progress >= 1) {
          this.removeListener('event:exporter', listener)
          res()
        }
      }
      this.on('event:exporter', listener)
    })
  }

  get networkInterface () {
    return this[fields].sess.networkInterface
  }

  get promiscuous () {
    return this[fields].sess.promiscuous
  }

  get snaplen () {
    return this[fields].sess.snaplen
  }

  get id () {
    return this[fields].sess.id
  }

  get options () {
    return this[fields].sess.options
  }

  get inspectorSessions () {
    const { inspector } = this[fields]
    if (inspector) {
      return inspector.sessions
    }
    return null
  }

  startPcap () {
    return this[fields].sess.startPcap()
  }

  stopPcap () {
    return this[fields].sess.stopPcap()
  }

  destroy () {
    return this[fields].sess.destroy()
  }

  getFilteredFrames (name, offset, length) {
    return this[fields].sess.getFilteredFrames(name, offset, length)
  }

  getFrames (offset, length) {
    const { sess } = this[fields]
    return sess.getFrames(offset, length)
  }

  setDisplayFilter (name, filter) {
    const { filterCompiler } = this[fields]
    const body = filterCompiler.compile(filter, { built: false }).linked
    return this[fields].sess.setDisplayFilter(name, body)
  }
}

class SessionFactory extends kit.SessionFactory {
  constructor () {
    super()
    this[fields] = {
      tasks: [],
      linkLayers: [],
      enableDebugSession: false,
      filterCompiler: null,
    }
  }

  create () {
    return Promise.all(this[fields].tasks).then(() => {
      for (const link of this[fields].linkLayers) {
        super.registerLinkLayer(link.link, link.id, link.name)
      }
      return new Session(super.create(), this[fields])
    })
  }

  get filterCompiler () {
    return this[fields].filterCompiler
  }

  set filterCompiler (val) {
    this[fields].filterCompiler = val
  }

  registerLinkLayer (layer) {
    this[fields].linkLayers.push(layer)
  }

  registerImporter (importer) {
    super.registerImporter(importer)
  }

  registerExporter (exporter) {
    super.registerExporter(exporter)
  }

  get enableDebugSession () {
    return this[fields].enableDebugSession
  }

  set enableDebugSession (flag) {
    this[fields].enableDebugSession = flag
  }

  registerDissector (dissector) {
    if (dissector.main.endsWith('.js')) {
      const task = promiseReadFile(dissector.main, 'utf8')
        .then((script) => {
          const func = `(function(module){${script}})`
          super.registerScriptDissector(func, dissector.type)
          return Promise.resolve()
        })
      this[fields].tasks.push(task)
    } else {
      super.registerDissector(dissector.main, dissector.type)
    }
  }
}

module.exports = {
  Layer: kit.Layer,
  Pcap: kit.Pcap,
  SessionFactory,
  Token: kit.Token,
  Reader: kit.Reader,
  StreamReader: kit.StreamReader,
  Testing: kit.Testing,
  FilterCompiler,
}
