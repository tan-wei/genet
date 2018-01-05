const kit = require('./plugkit')
const fs = require('fs')
const promisify = require('es6-promisify')
const EventEmitter = require('events')
const FilterCompiler = require('./filter')
const VirtualFrame = require('./frame')
const Inspector = require('./inspector')

const fields = Symbol('fields')
const promiseReadFile = promisify(fs.readFile)
class Session extends EventEmitter {
  constructor (sess, options) {
    super()
    this[fields] = Object.assign({
      sess,
      filterCompiler: new FilterCompiler(),
      frameCache: new Map(),
    }, options)

    if (options.inspect) {
      this[fields].inspector = new Inspector()
    }

    const filterCompiler = new FilterCompiler()
    for (const trans of options.transforms) {
      switch (trans.type) {
        case 'string':
          filterCompiler.registerStringTransform(trans.func)
          break
        case 'token':
          filterCompiler.registerTokenTransform(trans.func)
          break
        case 'ast':
          filterCompiler.registerAstTransform(trans.func)
          break
        case 'template':
          filterCompiler.registerTemplateTransform(trans.func)
          break
        default:
          throw new Error(`unknown transform type: ${trans.type}`)
      }
    }
    this[fields].filterCompiler = filterCompiler

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
    sess.setInspectorCallback((msg) => {
      console.log(msg)
    })
  }

  importFile (file) {
    return this[fields].sess.importFile(file)
  }

  exportFile (file, filter = '') {
    const { filterCompiler } = this[fields]
    const body = filterCompiler.compile(filter)
    return this[fields].sess.exportFile(file, body)
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
    const { sess, frameCache } = this[fields]
    return sess.getFrames(offset, length).map((frame, index) => {
      let wrapped = frameCache.get(offset + index)
      if (wrapped) {
        return wrapped
      }
      wrapped = new VirtualFrame(frame)
      frameCache.set(offset + index, wrapped)
      return wrapped
    })
  }

  setDisplayFilter (name, filter) {
    const { filterCompiler } = this[fields]
    const body = filterCompiler.compile(filter)
    return this[fields].sess.setDisplayFilter(name, body)
  }

  createFilter (filter) {
    const { filterCompiler } = this[fields]
    return {
      expression: filter,
      test: filterCompiler.compileFunction(filter),
    }
  }
}

class SessionFactory extends kit.SessionFactory {
  constructor () {
    super()
    this[fields] = {
      tasks: [],
      linkLayers: [],
      transforms: [],
      attributes: {},
      inspect: true,
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

  registerLinkLayer (layer) {
    this[fields].linkLayers.push(layer)
  }

  registerFilterTransform (trans) {
    this[fields].transforms.push(trans)
  }

  registerImporter (importer) {
    super.registerImporter(importer)
  }

  registerExporter (exporter) {
    super.registerExporter(exporter)
  }

  registerAttributes (attrs) {
    this[fields].attributes = Object.assign(this[fields].attributes, attrs)
  }

  registerDissector (dissector) {
    if (typeof dissector.main === 'string') {
      const task = promiseReadFile(dissector.main, 'utf8')
      .then((script) => {
        const func = `(function(module){${script}})`
        super.registerDissector(func, dissector.type)
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
}
