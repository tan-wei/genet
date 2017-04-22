const kit = require('bindings')('plugkit.node')
const esprima = require('esprima')
const {rollup} = require('rollup')
const EventEmitter = require('events')

const map = new WeakMap()
function internal (object) {
  if (!map.has(object)) {
    map.set(object, {})
  }
  return map.get(object)
}

function roll(script) {
  return rollup({
    entry: script,
    external: (id) => {
      return false
    },
    acorn: {
      ecmaVersion: 8
    },
    plugins: [],
    onwarn: (e) => {}
  }).then((bundle) => {
    const result = bundle.generate({
      format: 'cjs'
    });
    return result.code
  })
}

class Session extends EventEmitter {
  constructor(sess) {
    super()
    internal(this).sess = sess

    this.status = {
      capture: false
    };
    this.filter = {};
    this.frame = {
      frames: 0
    };

    sess.setStatusCallback((status) => {
      this.status = status;
      this.emit('status', status)
    })
    sess.setFilterCallback((filter) => {
      this.filter = filter;
      this.emit('filter', filter)
    })
    sess.setFrameCallback((frame) => {
      this.frame = frame;
      this.emit('frame', frame)
    })
    sess.setLoggerCallback((log) => {
      this.emit('log', log)
    })
  }

  get networkInterface() {
    return internal(this).sess.networkInterface
  }

  get promiscuous() {
    return internal(this).sess.promiscuous
  }

  get snaplen() {
    return internal(this).sess.snaplen
  }

  get id() {
    return internal(this).sess.id
  }

  get options() {
    return internal(this).sess.options
  }

  startPcap() {
    return internal(this).sess.startPcap()
  }

  stopPcap() {
    return internal(this).sess.stopPcap()
  }

  destroy() {
    return internal(this).sess.destroy()
  }

  getFilteredFrames(name, offset, length) {
    return internal(this).sess.getFilteredFrames(name, offset, length)
  }

  getFrames(offset, length) {
    return internal(this).sess.getFrames(offset, length)
  }

  setDisplayFilter(name, filter) {
    let body = ''
    const ast = esprima.parse(filter)
    switch (ast.body.length) {
      case 0:
        break
      case 1:
        const root = ast.body[0]
        if (root.type !== "ExpressionStatement")
          throw new SyntaxError()
        body = JSON.stringify(root.expression)
        break
      default:
        throw new SyntaxError()
    }
    return internal(this).sess.setDisplayFilter(name, body)
  }
}

class SessionFactory extends kit.SessionFactory {
  constructor() {
    super()
    internal(this).tasks = []
    internal(this).linkLayers = []
  }

  create() {
    return Promise.all(internal(this).tasks).then(() => {
      for (let link of internal(this).linkLayers) {
        super.registerLinkLayer(link.link, link.id, link.name)
      }
      return new Session(super.create())
    })
  }

  registerLinkLayer(layer) {
    internal(this).linkLayers.push(layer)
  }

  registerDissector(dissector) {
    if (typeof dissector === 'string') {
      let task = roll(dissector).then((script) => {
        super.registerDissector(script, dissector)
        return Promise.resolve()
      }).catch((err) => {
        return Promise.reject(err)
      })
      internal(this).tasks.push(task)
    } else {
      super.registerDissector(dissector)
    }
  }

  registerStreamDissector(dissector) {
    if (typeof dissector === 'string') {
      let task = roll(dissector).then((script) => {
        super.registerStreamDissector(script, dissector)
        return Promise.resolve()
      }).catch((err) => {
        return Promise.reject(err)
      })
      internal(this).tasks.push(task)
    } else {
      super.registerStreamDissector(dissector)
    }
  }
}

module.exports = {
  Layer: kit.Layer,
  Chunk: kit.Chunk,
  Pcap: kit.Pcap,
  SessionFactory: SessionFactory
}
