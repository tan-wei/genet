const { Token } = require('./plugkit')
const acorn = require('acorn')
const estraverse = require('estraverse')
const escodegen = require('escodegen')
const fs = require('fs')
const path = require('path')
const plugin = require('./acorn')
const vm = require('vm')

const fields = Symbol('fields')
const runtime = fs.readFileSync(path.join(__dirname, 'runtime.js'), 'utf8')
function makeOp (opcode, ...args) {
  return {
    type: 'CallExpression',
    callee: {
      type: 'Identifier',
      name: '__operator',
    },
    arguments: [{
      type: 'Literal',
      value: opcode,
    }].concat(args),
  }
}

class FilterCompiler {
  constructor () {
    this[fields] = { macros: [] }
  }

  set macros (macros) {
    this[fields].macros = macros
  }

  get macros () {
    return this[fields].macros
  }

  transpile (filter) {
    const { macros } = this[fields]
    if (!filter) {
      return { expression: '' }
    }

    acorn.plugins.filter = plugin
    let ast = acorn.parse(filter, { plugins: { filter: true } })
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
              type: 'UnaryExpression',
              operator: '!',
              argument: makeOp('!', node.test),
              prefix: true,
            }
            return node
          default:
        }
        if (node.extended === 'macro') {
          for (const macro of macros) {
            const result = macro.func(node.name)
            if (typeof result === 'string') {
              return acorn.parse(result)
                .body[0].expression
            }
          }
          throw new Error(`unrecognized macro: @${node.name}`)
        } else if (node.extended === 'attr') {
          let expr = acorn.parse(`__resolve(${Token.get(node.name)})`)
            .body[0].expression
          for (const func of (node.pipeline || [])) {
            expr = {
              type: 'CallExpression',
              callee: {
                type: 'Identifier',
                name: '__pipeline',
              },
              arguments: [
                {
                  type: 'Literal',
                  value: func.name,
                },
                expr
              ].concat(func.args),
            }
          }
          return expr
        }
      },
    })
    return { expression: escodegen.generate(ast) }
  }

  link (code) {
    if (code.expression === '') {
      return ''
    }
    return runtime
      .replace('@@expression@@', code.expression)
  }

  build (prog) {
    const options = { displayErrors: true }
    if (prog === '') {
      return (() => true)
    }
    return vm.runInThisContext(prog, options)
  }

  compile (filter, opt = {}) {
    const options = Object.assign({ built: true }, opt)
    const result = {
      filter,
      transpiled: this.transpile(filter),
    }
    result.linked = this.link(result.transpiled)
    if (options.built) {
      result.built = this.build(result.linked)
    }
    return result
  }
}

module.exports = FilterCompiler
