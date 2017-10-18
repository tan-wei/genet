#!/usr/bin/env node

const parser = require('./' + process.argv[2])
const snippetDir = process.argv[3]
const files = process.argv.slice(4, -1)
const out = process.argv[process.argv.length - 1]
const fs = require('fs')
const path = require('path')
const glob = require('glob')

const tasks = []
for (const file of files) {
  tasks.push(parser(file))
}

Promise.all(tasks).then(items => {
  const groups = group(uniqify(Array.prototype.concat.apply([], items)))
  fs.writeFileSync(out, markdown(groups))
})

function itemId(item) {
  const id = `${item.type}@${item.module}@${item.name}`
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, '-')
    .replace(/-([a-z0-9]+)-\1-/, '-$1-')
  return id
}

function uniqify(array) {
  const set = new Set()
  const unique = []
  for (const item of array) {
    const id = itemId(item)
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
    if (item.type === 'c-func' || item.type === 'js-func') {
      group.functions.push(item)
    } else if (item.type === 'js-prop') {
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

function snippet(id) {
  const files = glob.sync(path.join(snippetDir, id + '.*'))
  return files.map(f => {
    return {
      data: fs.readFileSync(f, 'utf8'),
      type: f.endsWith('.c') ? 'c' : 'javascript'
    }
  })
}

function markdown(groups) {
  let doc = '# Dissector API\n'
  for (const group of groups) {
    doc += `## ${group.name}\n`
    if (group.properties.length > 0) {
      doc += `### Properties\n`
      for (const prop of group.properties) {
        const id = itemId(prop)
        const link = '/#/' + path.basename(out) + '?id=' + id
        doc += `<h4 id=${id}><a href="${link}" class="anchor">#${prop.name}</a> `
        if (prop.propType) {
          doc += `\`${prop.propType}\` `
        }
        if (prop.readonly) {
          doc += `*readonly*`
        }
        doc += `</h4>\n\n`
        if (prop.comment !== null) {
          doc += prop.comment.paragraph + '\n\n'
        }
      }
    }
    if (group.functions.length > 0) {
      doc += `### Functions\n`
      for (const func of group.functions) {
        const ret = func.returnType
        const id = itemId(func)
        const link = '/#/' + path.basename(out) + '?id=' + id
        if (func.type === 'c-func') {
          const args = func.args
            .map(a => `\`${a.type}\` ${a.name}`)
            .join(', ')
          doc += `<h4 id="${id}"> \`${ret}\` <a href="${link}" class="anchor">${func.name}</a> `
          doc += `(${args})</h4>\n\n`
        } else if (func.type === 'js-func') {
          const args = func.args
            .map(a => a.defaultValue ? `${a.name} *= ${a.defaultValue}*` : a.name)
            .join(', ')
          doc += `<h4 id="${id}"><a href="${link}" class="anchor">#${func.name}</a> `
          doc += `(${args})`
          if (func.returnType) {
            doc += ` -> \`${func.returnType}\``
          }
          doc += `</h4>\n\n`
        }
        if (func.comment !== null) {
          doc += func.comment.paragraph + '\n\n'
          for (const command of func.comment.commands) {
            if (command.name === 'remark') {
              doc += `?> ${command.value}\n`
            }
          }
        }
        for (const code of snippet(itemId(func))) {
          doc += '```' + code.type + '\n'
          doc += code.data
          doc += '```\n'
        }
      }
    }
    doc += `\n\n`
  }
  return doc
}
