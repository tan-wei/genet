#!/usr/bin/env node

const estraverse = require('estraverse')
const esprima = require('esprima')
const fs = require('fs')
const {promisify} = require('util')

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

function parse(ast) {
  let className = ''
  let funcName = ''
  let staticFunc = false

  const items = []
  let item = {
    type: 'js-func',
    returnType: '',
    args: [],
    comment: null
  }

  estraverse.traverse(ast, {
    enter: (node, parent) => {
      switch (node.type) {
        case 'ClassDeclaration':
          item.module = node.id.name
          break
        case 'MethodDefinition':
          switch (node.kind) {
            case 'method':
              item.type = 'js-func'
              break
            case 'get':
              item.type = 'js-get'
              break
            case 'set':
              item.type = 'js-set'
              break
          }
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

  const properties = new Map()
  for (const item of items) {
    const key = `${item.module}#${item.name}`
    if (item.type === 'js-get' || item.type === 'js-set') {
      const prop = properties.get(key) || {type: 'js-prop', readonly: true}
      prop.module = prop.module || item.module
      prop.name = prop.name || item.name
      prop.comment = prop.comment || item.comment
      if (item.type === 'js-set') {
        prop.readonly = false
      }
      if (prop.comment !== null) {
        for (const command of prop.comment.commands) {
          if (command.name === 'property') {
            prop.propType = command.value
          }
        }
      }
      properties.set(key, prop)
    }
  }

  for (const prop of properties) {
    items.push(prop[1])
  }

  return items
}

module.exports = function(file) {
  return promisify(fs.readFile)(file, 'utf8').then(data => {
    return parse(esprima.parse(data, {attachComment: true, comment: true}))
  })
}
