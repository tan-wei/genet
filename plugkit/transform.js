const kit = require('bindings')('plugkit.node')
const estraverse = require('estraverse')
const esprima = require('esprima')

function makeOp(opcode, ...args) {
  return {
    type: 'CallExpression',
    callee: { type: 'Identifier', name: '$_op' },
    arguments: [ { type: "Literal", value: opcode } ].concat(args)
  }
}

module.exports = function transform(ast, transforms, attributes) {
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

          const id = identifiers.join('.') + '.'
          const attrs = Object.keys(attributes).map((key) => {
            if (key.startsWith('.')) {
              return identifiers[0] + key
            } else {
              return key
            }
          }).filter((key) => id.startsWith(key) && id[key.length] === '.')
          attrs.sort((a, b) => b.split('.').length - a.split('.').length)

          let property = identifiers.join('.')
          let attrName = ''
          if (attrs.length) {
            const first = attrs[0]
            if (first in attributes && attributes[first].virtual !== true) {
              attrName = first
            } else {
              attrName = '.' + first.split('.').slice(1).join('.')
            }
            property = property.slice(first.length)
          }

          let layer = identifiers[0]
          if (!(layer in attributes)) {
            layer = ''
          }

          let code = ''
          if (layer.length) {
            if (layer === attrName) {
              code = `$_frame.layer(${kit.Token.get(layer)})${property}`
            } else {
              code = `$_frame.layer(${kit.Token.get(layer)}).attr(${kit.Token.get(attrName)})${property}`
            }
          } else {
            code = attrName
          }
          return esprima.parse(code).body[0].expression

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
        if (node.name in attributes) {
          return esprima.parse(`$_frame.layer('${node.name}')`).body[0].expression
        }
      }
    }
  })

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

  for (const trans of transforms) {
    ast = trans.execute(ast)
  }

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
  return ast
}
