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
        default:
      }
    },
  })
}

function processTemplates (ast, templateTransforms) {
  return estraverse.replace(ast, {
    enter: (node, parent) => {
      let tag = ''
      let value = ''
      if (node.type === 'TemplateLiteral') {
        for (const quasi of node.quasis) {
          if (quasi.type === 'TemplateElement') {
            value += quasi.value.raw
          }
        }
        if (parent.type === 'TaggedTemplateExpression') {
          if (parent.tag.type === 'Identifier') {
            tag = parent.tag.name
          }
        }
        for (const trans of templateTransforms) {
          const code = trans(tag, value)
          if (typeof code === 'string' && code.length > 0) {
            return esprima.parse(code).body[0].expression
          }
        }
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
      macros: [],
      stringTransforms: [],
      tokenTransforms: [],
      astTransforms: [],
      templateTransforms: [],
    }
  }

  registerMacro (macro) {
    const { macros } = this[fields]
    macros.push(macro)
  }

  registerStringTransform (trans) {
    const { stringTransforms } = this[fields]
    stringTransforms.push(trans)
  }

  registerTokenTransform (trans) {
    const { tokenTransforms } = this[fields]
    tokenTransforms.push(trans)
  }

  registerAstTransform (trans) {
    const { astTransforms } = this[fields]
    astTransforms.push(trans)
  }

  registerTemplateTransform (trans) {
    const { templateTransforms } = this[fields]
    templateTransforms.push(trans)
  }

  compile (filter) {
    const {
      stringTransforms, tokenTransforms,
      astTransforms, templateTransforms, macros,
    } = this[fields]
    if (!filter) {
      return ''
    }
    let str = filter.replace(/@([^ ]+)(?: |$)/g, (match, exp) => {
      for (const macro of macros) {
        const result = macro(exp)
        if (typeof result === 'string') {
          return result
        }
      }
      throw new Error(`unrecognized macro: @${exp}`)
    })
    for (const trans of stringTransforms) {
      str = trans(str)
    }
    let tokens = esprima.tokenize(str)
    for (const trans of tokenTransforms) {
      tokens = trans(tokens)
    }
    let ast = esprima.parse(tokens.map((token) => token.value).join(' '))
    ast = processTemplates(ast, templateTransforms)
    tokens = esprima.tokenize(escodegen.generate(ast))
    tokens = processIdentifiers(tokens)
    ast = esprima.parse(tokens.map((token) => token.value).join(' '))
    for (const trans of astTransforms) {
      ast = trans(ast)
    }
    ast = ast.body[0].expression
    ast = processOperators(ast)
    ast = makeValue(ast)
    return runtime.replace('@@@', escodegen.generate(ast))
  }

  compileFunction (filter) {
    const options = { displayErrors: true }
    if (filter.length === 0) {
      return (() => true)
    }
    return vm.runInThisContext(this.compile(filter), options)
  }
}

module.exports = Filter
