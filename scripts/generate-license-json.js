#!/usr/bin/env node

const path = require('path')
const glob = require('glob')
const fs = require('fs')
const zlib = require('zlib')

let files = []
for (const dir of ['node_modules', 'plugkit']) {
  files = files.concat(glob.sync(path.join(dir, '**/LICENSE*')))
}

const contents = files.map((file) => {
  const name = path.basename(path.dirname(file))
  return {
    name,
    body: fs.readFileSync(file, 'utf8')
  }
})

const json = JSON.stringify(contents)
const out = fs.createWriteStream('genet_modules/src/asset/license.json.gz')
const gzip = zlib.createGzip({ level: zlib.constants.Z_BEST_COMPRESSION })
gzip.pipe(out)
gzip.end(json, 'utf8')
