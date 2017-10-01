#!/usr/bin/env node
const { promisify } = require('util')
const readFile = promisify(require('fs').readFile)
const writeFile = promisify(require('fs').writeFile)
const path = require('path')

const files = process.argv.slice(2)
if (files.length >= 2) {
  const inputs = files.slice(0, files.length - 1)
  const output = files[files.length - 1]
  const tasks = []
  for (const file of inputs) {
    tasks.push(readFile(file))
  }
  Promise.all(tasks).then((contents) => {
    const literals = contents.map((content) => `R""""(${content})""""`).join(',')
    const source = `namespace { const char * files[] = {${literals}}; }`
    writeFile(output, source)
  })
}

function path2var(file) {
  return path.basename(file).replace(/[^A-Za-z0-9]/g, '_')
}
