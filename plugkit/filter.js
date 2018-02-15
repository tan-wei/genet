const { Token } = require('./plugkit')
const estraverse = require('estraverse')
const esprima = require('esprima')
const escodegen = require('escodegen')
const fs = require('fs')
const path = require('path')
const vm = require('vm')

const fields = Symbol('fields')
const runtime = fs.readFileSync(path.join(__dirname, 'runtime.js'), 'utf8')
function makeValue (val) {
  return {
    type: 'CallExpression',
    callee: {
      type: 'Identifier',
      name: '__value',
    },
    arguments: [val],
  }
}

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

function processOperators (ast) {
  return estraverse.replace(ast, {
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
        case 'CallExpression':
          if (!(node.callee.name || '').startsWith('__')) {
            return makeOp('()', node.callee, ...node.arguments)
          }
          return node
        default:
      }
    },
  })
}

let counter = 0
function symbolName (base) {
  counter += 1
  return `__$${base.replace(/[^0-9a-zA-Z]/g, '_')}_${counter}`
}

function processArrays (ast, globals) {
  return estraverse.replace(ast, {
    enter: (node) => {
      if (node.type === 'ArrayExpression') {
        if (node.elements.every((elem) =>
          elem.type === 'Literal' && typeof elem.value === 'number')) {
          const sym = symbolName('array')
          const literal = node.elements.map((elem) => elem.value).join(',')
          globals.push(`const ${sym} = [${literal}];`)
          return {
            'type': 'Identifier',
            'name': sym,
          }
        }
      }
    },
  })
}

function processIdentifiers (tokens, attrs, globals) {
  function resolve (identifiers, resolvedTokens) {
    resolvedTokens.push(
      {
        type: 'Identifier',
        value: '__resolve',
      },
      {
        type: 'Punctuator',
        value: '(',
      }
    )
    let index = identifiers.length
    for (; index > 0; index -= 1) {
      const token = identifiers.slice(0, index).join('.')
      if (token in attrs && attrs[token].property !== true) {
        break
      }
    }
    if (index < 0) {
      index = identifiers.length
    }
    const token = identifiers.slice(0, index).join('.')
    const sym = symbolName(token)
    globals.push(`const ${sym} = ${Token.get(token)};`)
    resolvedTokens.push(
      {
        type: 'Identifier',
        value: sym,
      }
    )
    for (const id of identifiers.slice(index)) {
      resolvedTokens.push(
        {
          type: 'Punctuator',
          value: ',',
        },
        {
          type: 'String',
          value: JSON.stringify(id),
        }
      )
    }
    resolvedTokens.push({
      type: 'Punctuator',
      value: ')',
    })
  }

  const resolvedTokens = []
  let identifiers = []
  for (const token of tokens) {
    if (token.type === 'Identifier') {
      identifiers.push(token.value)
    } else if (identifiers.length === 0 || token.value !== '.') {
      if (identifiers.length > 0) {
        resolve(identifiers, resolvedTokens)
        identifiers = []
      }
      resolvedTokens.push(token)
    }
  }
  if (identifiers.length > 0) {
    resolve(identifiers, resolvedTokens)
  }
  return resolvedTokens
}

class Filter {
  constructor () {
    this[fields] = {
      macros: [],
      attrs: {},
      macroPrefix: '@',
    }
  }

  get macroPrefix () {
    return this[fields].macroPrefix
  }

  set macroPrefix (val) {
    this[fields].macroPrefix = val
  }

  set macros (macros) {
    this[fields].macros = macros
  }

  get macros () {
    return this[fields].macros
  }

  set attrs (attrs) {
    this[fields].attrs = attrs
  }

  get attrs () {
    return this[fields].attrs
  }

  compile (filter) {
    const { macros, attrs, macroPrefix } = this[fields]
    if (!filter) {
      return {
        expression: '',
        globals: [],
      }
    }
    const pattern = new RegExp(`(${macroPrefix})([^ ]+)(?: |$)`, 'g')
    const str = filter.replace(pattern, (match, prefix, exp) => {
      for (const macro of macros) {
        const result = macro.func(exp)
        if (typeof result === 'string') {
          return result
        }
      }
      throw new Error(`unrecognized macro: ${prefix}${exp}`)
    })
    const globals = []
    const tokens = processIdentifiers(esprima.tokenize(str), attrs, globals)
    const tree = esprima.parse(tokens.map((token) => token.value).join(' '))
    const ast = makeValue(
      processArrays(processOperators(tree.body[0].expression), globals))
    return {
      expression: escodegen.generate(ast),
      globals,
    }
  }

  link (code) {
    if (code.expression === '') {
      return ''
    }
    return runtime
      .replace('@@globals@@', code.globals.join('\n'))
      .replace('@@expression@@', code.expression)
  }

  build (prog) {
    const options = { displayErrors: true }
    if (prog === '') {
      return (() => true)
    }
    return vm.runInThisContext(prog, options)
  }
}

module.exports = Filter
