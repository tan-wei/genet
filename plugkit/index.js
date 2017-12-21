const kit = require('bindings')('plugkit.node')
const fs = require('fs')
const promisify = require('es6-promisify')
const EventEmitter = require('events')
const Filter = require('./filter')

const fields = Symbol('fields')
const promiseReadFile = promisify(fs.readFile)
function compileFilter (filter, transforms) {
  const compiler = new Filter()
  for (const trans of transforms) {
    switch (trans.type) {
      case 'string':
        compiler.registerStringTransform(trans.func)
        break
      case 'token':
        compiler.registerTokenTransform(trans.func)
        break
      case 'ast':
        compiler.registerAstTransform(trans.func)
        break
      case 'template':
        compiler.registerTemplateTransform(trans.func)
        break
      default:
        throw new Error(`unknown transform type: ${trans.type}`)
    }
  }
  return compiler.compile(filter)
}

class Session extends EventEmitter {
  constructor (sess, options) {
    super()
    this[fields] = Object.assign({ sess }, options)

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
  }

  exportFrames (file, callback, filter = '', type = '') {
    this[fields].sess.exportFrames(file, callback, filter, type)
  }

  importFrames (file, callback, type = '') {
    this[fields].sess.importFrames(file, callback, type)
  }

  importFile (file) {
    return this[fields].sess.importFile(file)
  }

  exportFile (file, filter = '') {
    const body = compileFilter(filter, this[fields].transforms)
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
    return this[fields].sess.getFrames(offset, length)
  }

  setDisplayFilter (name, filter) {
    const body = compileFilter(filter, this[fields].transforms)
    return this[fields].sess.setDisplayFilter(name, body)
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
  }
