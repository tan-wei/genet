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
  let item = {
    type: 'js-function',
    returnType: '',
    args: [],
    comment: null
  }

  function parseComments(comments) {
    if (!Array.isArray(comments)) {
      return null
    }
    const commands = []
    let paragraph = ''
    for (const comment of comments) {
      const line = comment.value.trim()
      if (line.startsWith('@')) {
        const result = line.match(/@(\w+)( .+)?$/)
        commands.push({
          name: result[1],
          value: result[2].trim(),
        })
      } else {
        paragraph += line + '\n'
      }
    }
    return {
      paragraph: paragraph.trim(),
      commands
    }
  }

  function parseParams(params) {
    const args = []
    if (!Array.isArray(params)) {
      return args
    }
    for (const param of params) {
      if (param.type === 'Identifier') {
        args.push({name: param.name})
      } else if (param.type === 'AssignmentPattern') {
        if (param.right.type === 'Literal') {
          args.push({name: param.left.name, defaultValue: param.right.raw})
        }
      }
    }
    return args
  }

  estraverse.traverse(ast, {
    enter: (node, parent) => {
      switch (node.type) {
        case 'ClassDeclaration':
          item.type = 'js-function'
          item.module = node.id.name
          break
        case 'MethodDefinition':
          item.name = node.key.name
          item.staticFunc = node.static
          item.comment = parseComments(node.leadingComments)
          item.returnType = ''
          if (item.comment !== null) {
            for (const command of item.comment.commands) {
              if (command.name === 'return') {
                item.returnType = command.value
              }
            }
          }
          break
        case 'FunctionExpression':
          item.args = parseParams(node.params)
          if (item.module && item.name) {
            items.push(Object.assign({}, item))
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
    return parse(esprima.parse(data, {attachComment: true, comment: true}))
  })
}
