const estraverse = require('estraverse')

function makeOp(opcode, ...args) {
  return {
    type: 'CallExpression',
    callee: { type: 'Identifier', name: '$_op' },
    arguments: [ { type: "Literal", value: opcode } ].concat(args)
  }
}

module.exports = function transform(ast, transforms) {
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
          type: "LogicalExpression",
          operator: "||",
          left: {
            type: "CallExpression",
            callee: {
              type: "MemberExpression",
              object: { type: "Identifier", name: "$_frame" },
              property: { type: "Identifier", name: "layer" }
            },
            arguments: [ { type: "Literal", value: node.name } ]
          },
          right: {
            type: "MemberExpression",
            computed: true,
            object: {
              type: "Identifier",
              name: "this"
            },
            property: {
              type: "Literal",
              value: node.name
            }
          }
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
