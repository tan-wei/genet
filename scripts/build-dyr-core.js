#!/usr/bin/env node

const path = require('path')
const execa = require('execa')
const glob = require('glob')
const mkpath = require('mkpath')
const fs = require('fs')

const src = path.resolve(__dirname, '../dyr-core')
const rustSrc = path.resolve(src, 'dyr_kernel')
const nodeSrc = path.resolve(src, 'dyr_node')
const scriptFiles = glob.sync(path.resolve(nodeSrc, '*.{js,json}'))
const dst = path.resolve(__dirname, '../deplug-modules/dyr_node')
const dstBin = path.resolve(__dirname, '../deplug-modules/bin/dyr_node')

const env = require('./plugkit-npm-env')

mkpath.sync(dst)
mkpath.sync(dstBin)

async function exec() {
  await execa.shell('cargo build --release', {
    cwd: rustSrc,
    stdio: 'inherit'
  })
  
  await execa('node-gyp', ['configure'], { env, cwd: nodeSrc, stdio: 'inherit' })
  await execa('node-gyp', ['build'], { env, cwd: nodeSrc, stdio: 'inherit' })

  const binaryFiles = glob.sync(path.resolve(nodeSrc, 'build/Release/*.{node,lib}'))
  for (const file of binaryFiles) {
    fs.createReadStream(file)
      .pipe(fs.createWriteStream(
        path.resolve(dstBin, path.basename(file))))
  }

  for (const file of scriptFiles) {
    fs.createReadStream(file)
      .pipe(fs.createWriteStream(
        path.resolve(dst, path.basename(file))))
  }
}

exec()