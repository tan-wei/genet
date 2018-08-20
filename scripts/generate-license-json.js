#!/usr/bin/env node

const path = require('path')
const glob = require('glob')
const fs = require('fs-extra')
const zlib = require('zlib')
const { promisify } = require('util')
const promiseGlob = promisify(glob)

let files = promiseGlob(path.join('node_modules/**/LICENSE*'))

const { dependencies } = fs.readJsonSync('package-lock.json')

const contents = files.then((files) => {
  return Promise.all(files.map(async (file) => {
    try {
      const pkg = await fs.readJson(path.join(path.dirname(file), 'package.json'))
      const dep = dependencies[pkg.name]
      if (!dep || dep.dev) {
        return null
      }
      const body = await fs.readFile(file, 'utf8')
      return {
        name: pkg.name,
        body,
      }
    } catch (err) {
      return null
    }
  }))
})

contents.then((data) => {

  const json = JSON.stringify(data.filter((item) => item))
  const out = fs.createWriteStream('genet_modules/src/asset/license.json.gz')
  const gzip = zlib.createGzip({ level: zlib.constants.Z_BEST_COMPRESSION })
  gzip.pipe(out)
  gzip.end(json, 'utf8')
})
