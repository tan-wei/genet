#!/usr/bin/env node
const { promisify } = require('util')
const readFile = promisify(require('fs').readFile)
const writeFile = promisify(require('fs').writeFile)
const uglify = require('uglify-es')
const path = require('path')

const files = process.argv.slice(2)
if (files.length >= 1) {
  const inputs = files.slice(0, files.length - 1)
  const output = files[files.length - 1]
  const tasks = []
  for (const file of inputs) {
    tasks.push(readFile(file, 'utf8'))
  }
  Promise.all(tasks).then((contents) => {
    const literals = contents
      .map((content) => {
        const minified = uglify.minify(content).code
        return `R""""((function(exports){${minified}}))""""`
      }).join(',')
    const source = `namespace { const char * files[] = {${literals}}; }`
    writeFile(output, source)
  })
}

function path2var(file) {
  return path.basename(file).replace(/[^A-Za-z0-9]/g, '_')
}
