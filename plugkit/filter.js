const estraverse = require('estraverse')
const esprima = require('esprima')
const escodegen = require('escodegen')
const fs = require('fs')
const path = require('path')

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
        default:
      }
    },
  })
}

function processIdentifiers (tokens) {
  const resolvedTokens = []
  let identifiers = []
  for (const token of tokens) {
    if (token.type === 'Identifier') {
      identifiers.push(token.value)
    } else if (identifiers.length === 0 || token.value !== '.') {
      if (identifiers.length > 0) {
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
        for (const id of identifiers) {
          resolvedTokens.push(
            {
              type: 'String',
              value: JSON.stringify(id),
            },
            {
              type: 'Punctuator',
              value: ',',
            }
          )
        }
        resolvedTokens.pop()
        resolvedTokens.push({
          type: 'Punctuator',
          value: ')',
        })
        identifiers = []
      }
      resolvedTokens.push(token)
    }
  }
  if (identifiers.length > 0) {
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
    for (const id of identifiers) {
      resolvedTokens.push(
        {
          type: 'String',
          value: JSON.stringify(id),
        },
        {
          type: 'Punctuator',
          value: ',',
        }
      )
    }
    resolvedTokens.pop()
    resolvedTokens.push({
      type: 'Punctuator',
      value: ')',
    })
  }
  return resolvedTokens
}

class Filter {
  constructor () {
    this[fields] = {
      stringTransforms: new Set(),
      tokenTransforms: new Set(),
      astTransforms: new Set(),
    }
  }

  compile (filter) {
    const { stringTransforms, tokenTransforms, astTransforms } = this[fields]
    if (!filter) {
      return ''
    }
    let str = filter
    for (const trans of stringTransforms) {
      str = trans(str)
    }
    let tokens = esprima.tokenize(str)
    for (const trans of tokenTransforms) {
      tokens = trans(tokens)
    }
    tokens = processIdentifiers(tokens)
    let ast = esprima.parse(tokens.map((token) => token.value).join(' '))
    for (const trans of astTransforms) {
      ast = trans(ast)
    }
    ast = ast.body[0].expression
    ast = processOperators(ast)
    ast = makeValue(ast)
    return runtime.replace('@@@', escodegen.generate(ast))
  }
}

module.exports = Filter
