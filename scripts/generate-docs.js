#!/usr/bin/env node

const parser = require('./' + process.argv[2])
const files = process.argv.slice(3, -1)
const out = process.argv[process.argv.length - 1]
const fs = require('fs')

const tasks = []
for (const file of files) {
  tasks.push(parser(file))
}

Promise.all(tasks).then(items => {
  const groups = group(uniqify(Array.prototype.concat.apply([], items)))
  fs.writeFileSync(out, markdown(groups))
})

function uniqify(array) {
  const set = new Set()
  const unique = []
  for (const item of array) {
    const id = `${item.type}@${item.module}@${item.name}`
    if (!set.has(id)) {
      unique.push(item)
      set.add(id)
    }
  }
  return unique
}

function group(array) {
  const groups = []
  const map = new Map()
  for (const item of array) {
    let group = {
      name: item.module,
      functions: []
    }
    if (map.has(item.module)) {
      group = map.get(item.module)
    } else {
      map.set(item.module, group)
    }
    if (item.type === 'c-function' || item.type === 'js-function') {
      group.functions.push(item)
    }
  }
  for (const pair of map) {
    groups.push(pair[1])
  }
  groups.sort((a, b) => {
    if (a.name < b.name) return -1
    if (a.name > b.name) return 1
    return 0
  })
  return groups
}

function markdown(groups) {
  let doc = '# Dissector API\n'
  for (const group of groups) {
    doc += `## ${group.name}\n`
    if (group.functions.length > 0) {
      doc += `### Functions\n`
      for (const func of group.functions) {
        const args = func.args
          .map(a => `\`${a.type}\` ${a.name}`)
          .join(', ')
        const ret = func.returnType
        if (func.type === 'c-function') {
          doc += `#### \`${ret}\` ${func.name} `
        } else if (func.type === 'js-function') {
          doc += `#### ${func.module}#${func.name} `
        }
        doc += `(${args})\n\n`
        if (func.comment !== null) {
          doc += func.comment.paragraph + '\n\n'
        }
      }
    }
    doc += `\n\n`
  }
  return doc
}
