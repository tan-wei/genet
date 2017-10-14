#!/usr/bin/env node

const estraverse = require('estraverse')
const esprima = require('esprima')
const fs = require('fs')
const {promisify} = require('util')

function parse(ast) {
  let className = ''
  let funcName = ''
  let staticFunc = false

  const items = []

  estraverse.traverse(ast, {
    enter: (node, parent) => {
      switch (node.type) {
        case 'ClassDeclaration':
          className = node.id.name
          break
        case 'MethodDefinition':
          funcName = node.key.name
          staticFunc = node.static
          break
        case 'FunctionExpression':
          if (className.length > 0 && funcName.length > 0) {
            items.push({
              type: 'js-function',
              module: className,
              name: funcName,
              returnType: '',
              args: [],
              comment: null
            })  
          }
          break
      }
    },
    leave: (node, parent) => {
      switch (node.type) {
        case 'ClassDeclaration':
          className = ''
          break
      }
    }
  })
  return items
}

module.exports = function(file) {
  return promisify(fs.readFile)(file, 'utf8').then(data => {
    return parse(esprima.parse(data))
  })
}
