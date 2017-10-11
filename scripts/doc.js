#!/usr/bin/env node

const execa = require('execa')

function ast(filename) {
  return execa('clang', ['-cc1', '-ast-dump', filename], {reject: false}).then(result => {
    const regex = /^[`\| ]([`| -]*)(\w+) 0x[0-9a-f]+[ $](.*)$/mg
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

ast(process.argv[2]).then(ast => console.dir(ast))
