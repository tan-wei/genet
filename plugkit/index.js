const kit = require('bindings')('plugkit.node')
const fs = require('fs')
const path = require('path')
const esprima = require('esprima')
const estraverse = require('estraverse')
const escodegen = require('escodegen')
const {rollup} = require('rollup')
const EventEmitter = require('events')

const filterScript = fs.readFileSync(path.join(__dirname, 'filter.js'))

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
  constructor(sess, transforms) {
    super()
    internal(this).sess = sess
    internal(this).transforms = transforms

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
    let ast = esprima.parse(filter)

    for (const trans of internal(this).transforms) {
      ast = trans.execute(ast)
    }

    ast = estraverse.replace(ast, {
      enter: (node) => {
        if (node.type === 'MemberExpression' && !node.computed) {
          let child = node.object
          const identifiers = [node.property.name]
          while (child.type === 'MemberExpression' && !child.computed) {
            identifiers.unshift(child.property.name)
            child = child.object
          }
          if (child.type === 'Identifier' && child.name !== '$_frame') {
            identifiers.unshift(child.name)
            return {
              type: "CallExpression",
              callee: {
                  type: "MemberExpression",
                  object: { type: "Identifier", name: "$_frame" },
                  property: { type: "Identifier", name: "attr" }
              },
              arguments: [ { type: "Literal", value: identifiers.join('.') } ]
            }
          }
        }
      }
    })

    ast = estraverse.replace(ast, {
      enter: (node, parent) => {
        if (node.type === 'Identifier' && parent.type !== 'MemberExpression') {
          return {
            type: "CallExpression",
            callee: {
                type: "MemberExpression",
                object: { type: "Identifier", name: "$_frame" },
                property: { type: "Identifier", name: "layer" }
            },
            arguments: [ { type: "Literal", value: node.name } ]
          }
        }
      }
    })

    function makeOp(opcode, ...args) {
      return {
        type: 'CallExpression',
        callee: { type: 'Identifier', name: '$_op' },
        arguments: [ { type: "Literal", value: opcode } ].concat(args)
      }
    }

    ast = estraverse.replace(ast, {
      enter: (node) => {
        switch (node.type) {
          case 'BinaryExpression':
          case 'LogicalExpression':
            return makeOp(node.operator, node.left, node.right)
          case 'UnaryExpression':
            return makeOp(node.operator, node.argument)
          case 'ConditionalExpression':
            node.test = {
              type: "UnaryExpression",
              operator: "!",
              argument: makeOp('!', node.test),
              prefix: true
            }
            return node
          default:
        }
      }
    })

    if (ast.body.length) {
      ast = {
        type: "Program",
        body: [
          {
            type: "ExpressionStatement",
            expression: {
              type: "FunctionExpression",
              params: [
                { type: "Identifier", name: "$_frame" }
              ],
              body: {
                type: "BlockStatement",
                body: [
                  {
                    type: "ReturnStatement",
                    argument: {
                      type: "UnaryExpression",
                      operator: "!",
                      argument: makeOp('!', ast.body[0].expression),
                      prefix: true
                    }
                  }
                ]
              }
            }
          }
        ]
      }
    }

    const body = ast.body.length ? (filterScript + escodegen.generate(ast)) : ''
    console.log(body)
    return internal(this).sess.setDisplayFilter(name, body)
  }
}

class SessionFactory extends kit.SessionFactory {
  constructor() {
    super()
    internal(this).tasks = []
    internal(this).linkLayers = []
    internal(this).transforms = []
  }

  create() {
    return Promise.all(internal(this).tasks).then(() => {
      for (let link of internal(this).linkLayers) {
        super.registerLinkLayer(link.link, link.id, link.name)
      }
      return new Session(super.create(), internal(this).transforms)
    })
  }

  registerLinkLayer(layer) {
    internal(this).linkLayers.push(layer)
  }

  registerFilterTransform(trans) {
    internal(this).transforms.push(trans)
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
