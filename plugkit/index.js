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
  constructor(sess, literals) {
    super()
    internal(this).sess = sess
    internal(this).literals = literals

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

  analyze(frames) {
    return internal(this).sess.analyze(frames)
  }

  setDisplayFilter(name, filter) {
    let body = ''
    const tokens = esprima.tokenize(filter, {range: true})

    const replaceLiterals = (tokens) => {
      let array = []
      for (let i = 0; i < tokens.length; ++i) {
        if (tokens[i].type === 'Template') {
          let id = null
          if (i > 0 && tokens[i - 1].type === 'Identifier') {
            id = tokens[i - 1].value
            array.pop()
          }
          const token = tokens[i]
          const literals = internal(this).literals
          const value = token.value.substr(1, token.value.length - 2)
          let parser = null
          if (id && id in literals) {
            parser = literals[id]
          } else {
            for (const key in literals) {
              const lit = literals[key]
              if (lit.regexp && lit.regexp.test(value)) {
                parser = lit
                break
              }
            }
          }
          if (parser !== null) {
            const code = parser.parse(value)
            if (typeof code === 'string') {
              array = array.concat(esprima.tokenize(code))
            }
          }
        } else {
          array.push(tokens[i])
        }
      }
      return array
    }

    const mergeProperties = (tokens) => {
      let array = []
      let property = []
      for (let i = 0; i < tokens.length; ++i) {
        if (tokens[i].type === 'Identifier') {
          if (property.length === 0 || property[property.length - 1].range[1] === tokens[i].range[0]) {
            property.push(tokens[i])
            continue;
          }
        } else if (tokens[i].type === 'Punctuator') {
          if (property.length && property[property.length - 1].range[1] === tokens[i].range[0]) {
            property.push(tokens[i])
            continue;
          }
        }
        if (property.length >= 3) {
          array.push({type: 'Identifier', value: property[0].value})
          const name = property
            .filter(p => p.type === 'Identifier')
            .map(p => p.value)
            .join('.')
          array.push({type: 'Punctuator', value: '['})
          array.push({type: 'String', value: JSON.stringify(name)})
          array.push({type: 'Punctuator', value: ']'})
        } else {
          for (const prop of property) {
            array.push(prop)
          }
        }
        property = []
        array.push(tokens[i])
      }
      if (property.length >= 3) {
        array.push({type: 'Identifier', value: property[0].value})
        const name = property
          .filter(p => p.type === 'Identifier')
          .map(p => p.value)
          .join('.')
        array.push({type: 'Punctuator', value: '['})
        array.push({type: 'String', value: JSON.stringify(name)})
        array.push({type: 'Punctuator', value: ']'})
      } else {
        for (const prop of property) {
          array.push(prop)
        }
      }
      return array
    }

    const ast = esprima.parse(
      replaceLiterals(tokens).map(t => t.value).join(' '))

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
    internal(this).literals = {}
  }

  create() {
    return Promise.all(internal(this).tasks).then(() => {
      for (let link of internal(this).linkLayers) {
        super.registerLinkLayer(link.link, link.id, link.name)
      }
      return new Session(super.create(), internal(this).literals)
    })
  }

  registerLinkLayer(layer) {
    internal(this).linkLayers.push(layer)
  }

  registerFilterLiteral(literal) {
    internal(this).literals[literal.id] = literal
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
}

const tokenMap = {}
const tokenReverseMap = {}

class Token {
  static get(str) {
    if (str in tokenMap) {
      return tokenMap[str]
    }
    const id = kit.Token.get(str)
    tokenMap[str] = id
    return id
  }

  static string(id) {
    if (id in tokenReverseMap) {
      return tokenReverseMap[id]
    }
    const str = kit.Token.string(id)
    if (str) {
      tokenReverseMap[id] = str
      return str
    }
    return ''
  }
}

module.exports = {
  Layer: kit.Layer,
  Pcap: kit.Pcap,
  SessionFactory,
  Token
}
