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
      functions: [],
      properties: []
    }
    if (map.has(item.module)) {
      group = map.get(item.module)
    } else {
      map.set(item.module, group)
    }
    if (item.type === 'c-function' || item.type === 'js-function') {
      group.functions.push(item)
    } else if (item.type === 'js-property') {
      group.properties.push(item)
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
    if (group.properties.length > 0) {
      doc += `### Properties\n`
      for (const prop of group.properties) {
        doc += `#### #${prop.name} `
        if (prop.propType) {
          doc += `\`${prop.propType}\` `
        }
        if (prop.readonly) {
          doc += `*readonly*`
        }
        doc += `\n\n`
        if (prop.comment !== null) {
          doc += prop.comment.paragraph + '\n\n'
        }
      }
    }
    if (group.functions.length > 0) {
      doc += `### Functions\n`
      for (const func of group.functions) {
        const ret = func.returnType
        if (func.type === 'c-function') {
          const args = func.args
            .map(a => `\`${a.type}\` ${a.name}`)
            .join(', ')
          doc += `#### \`${ret}\` ${func.name} `
          doc += `(${args})\n\n`
        } else if (func.type === 'js-function') {
          const args = func.args
            .map(a => a.defaultValue ? `${a.name} *= ${a.defaultValue}*` : a.name)
            .join(', ')
          doc += `#### #${func.name} `
          doc += `(${args})`
          if (func.returnType) {
            doc += ` -> \`${func.returnType}\``
          }
          doc += `\n\n`
        }
        if (func.comment !== null) {
          doc += func.comment.paragraph + '\n\n'
        }
      }
    }
    doc += `\n\n`
  }
  return doc
}
