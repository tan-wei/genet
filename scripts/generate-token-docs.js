#!/usr/bin/env node

const files = process.argv.slice(2, -1)
const out = process.argv[process.argv.length - 1]
const fs = require('fs')
const jsonfile = require('jsonfile')
const {promisify} = require('util')

const tasks = []
for (const file of files) {
  tasks.push(promisify(jsonfile.readFile)(file))
}

function idType(id) {
  switch (id[0]) {
    case '@':
      return 'type'
    case '!':
      return 'error'
  }
  if (!id.includes('.')) {
    return 'protocol'
  }
  return 'attr'
}

function table(type, tokens) {
  let doc = '|ID|Name|Description|\n'
  doc += '|---|---|---|\n'

  const keys = Object.keys(tokens)
  keys.sort()

  for (const key of keys) {
    if (idType(key) === type) {
      doc += `|\`${key}\`|${tokens[key].name}| - |\n`
    }
  }
  return doc + '\n'
}

Promise.all(tasks).then(items => {
  const tokens = Object.assign({}, ...items)

  let doc = '# Well-known tokens\n'

  doc += '## Protocols\n'
  doc += table('protocol', tokens)

  doc += '## Types\n'
  doc += table('type', tokens)

  doc += '## Errors\n'
  doc += table('error', tokens)

  doc += '## Attributes\n'
  doc += table('attr', tokens)

  fs.writeFileSync(out, doc)
})
