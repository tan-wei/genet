const kit = require('bindings')('plugkit.node')
const { rollup } = require('rollup')
const EventEmitter = require('events')
const Filter = require('./filter')

const fields = Symbol('fields')
function roll (script) {
  return rollup({
    entry: script,
    external: () => false,
    acorn: { ecmaVersion: 8 },
    plugins: [],
    onwarn: () => null,
  }).then((bundle) => {
    const result = bundle.generate({ format: 'cjs' })
    return result.code
  })
}

class Session extends EventEmitter {
  constructor (sess, options) {
    super()
    this[fields] = {
      sess,
      transforms: options.transforms,
      attributes: options.attributes,
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
  }

  exportFrames (file, callback, filter = '', type = '') {
    this[fields].sess.exportFrames(file, callback, filter, type)
  }

  importFrames (file, callback, type = '') {
    this[fields].sess.importFrames(file, callback, type)
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

  analyze (frames) {
    return this[fields].sess.analyze(frames)
  }

  setDisplayFilter (name, filter) {
    const compiler = new Filter()
    for (const trans of this[fields].transforms) {
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
        default:
          throw new Error(`unknown transform type: ${trans.type}`)
      }
    }
    const body = compiler.compile(filter)
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
        importers: [],
        exporters: [],
        attributes: {},
      }
    }

    create () {
      return Promise.all(this[fields].tasks).then(() => {
        for (const link of this[fields].linkLayers) {
          super.registerLinkLayer(link.link, link.id, link.name)
        }
        return new Session(super.create(), {
          transforms: this[fields].transforms,
          attributes: this[fields].attributes,
        })
      })
    }

    registerLinkLayer (layer) {
      this[fields].linkLayers.push(layer)
    }

    registerFilterTransform (trans) {
      this[fields].transforms.push(trans)
    }

    registerImporter (importer) {
      this[fields].importers.push(importer)
    }

    registerExporter (exporter) {
      this[fields].exporters.push(exporter)
    }

    registerAttributes (attrs) {
      this[fields].attributes = Object.assign(this[fields].attributes, attrs)
    }

    registerDissector (dissector) {
      if (typeof dissector.main === 'string') {
        const task = roll(dissector.main).then((script) => {
          const func = `(function(module){${script}})`
          super.registerDissector(func, dissector.type)
          return Promise.resolve()
        })
        .catch((err) => Promise.reject(err))
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
