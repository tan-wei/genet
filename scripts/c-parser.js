#!/usr/bin/env node

const execa = require('execa')

function ast(filename) {
  return execa('clang', ['-cc1', '-ast-dump', filename], {reject: false}).then(result => {
    const regex = /^[`\| ]([`| -]*)(\w+) 0x[0-9a-f]+ (.*)$/mg
    let metches = null

    const stack = [{
      name: 'TranslationUnitDecl',
      children: []
    }]

    while ((metches = regex.exec(result.stdout)) !== null) {
      const depth = ((metches[1].match(/[ |`-]/g) || []).length + 1) / 2 + 1
      const name = metches[2]
      const value = metches[3]

      const node = {
        name,
        value,
        children: []
      }

      if (depth <= stack.length) {
        while (depth - 1 < stack.length) {
          stack.pop()
        }
      }
      stack[stack.length - 1].children.push(node)
      stack.push(node)
    }
    return stack[0].children
  })
}

function parseFullComment(decl) {
  let paragraph = ''
  const commands = []
  for (const item of decl.children) {
    if (item.name === 'ParagraphComment') {
      if (commands.length > 0) commands += '\n'
      for (const text of item.children) {
        const match = text.value.match(/Text=" (.*)"$/)
        if (match !== null) {
          paragraph += match[1] + '\n'
        }
      }
    } else if (item.name === 'BlockCommandComment') {
      const match = item.value.match(/Name="(.+)"/)
      if (match !== null) {
        const name = match[1]
        let value = ''
        for (const para of item.children) {
          if (value.length > 0) value += '\n'
          for (const text of para.children) {
            const match = text.value.match(/Text=" (.*)"$/)
            if (match !== null) {
              value += match[1] + '\n'
            }
          }
        }
        commands.push({
          name,
          value: value.trim()
        })
      }
    }
  }
  return {
    paragraph: paragraph.trim(),
    commands
  }
}

function parseFunctionDecl(decl) {
  if (decl.value.includes(' implicit used ')) {
    return null
  }
  const result = decl.value.match(/ (\w+) '(.+)\((.*)\)'(?: |$)/)
  const name = result[1].trim()
  const returnType = result[2].trim()
  const args = result[3].split(',').map(t => t.trim()).filter(t => t)
  if (name.endsWith('_')) {
    return null
  }
  const module = name.split('_')[0]
  let comment = null
  for (const item of decl.children) {
    if (item.name === 'FullComment') {
      comment = parseFullComment(item)
    }
  }
  return {
    type: 'c-function',
    module,
    name,
    returnType,
    args,
    comment
  }
}

module.exports = function(file) {
  return ast(file).then(ast => {
    const items = []
    for (const decl of ast) {
      let item = null
      switch (decl.name) {
        case 'FunctionDecl':
          if (item = parseFunctionDecl(decl)) {
            items.push(item)
          }
          break
      }
    }
    return items
  })
}
