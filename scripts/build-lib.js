#!/usr/bin/env node

const path = require('path')
const execa = require('execa')
const glob = require('glob')
const fs = require('fs-extra')

const src = path.resolve(__dirname, '../lib')
const rustSrc = path.resolve(src, 'genet-kernel')
const nodeSrc = path.resolve(src, 'genet-node')
const scriptFiles = glob.sync(path.resolve(nodeSrc, '*.{js,json}'))
const dst = path.resolve(__dirname, '../genet_modules/genet-node')
const dstBin = path.resolve(__dirname, '../genet_modules/bin/genet-node')

const env = require('./npm-env')

fs.ensureDirSync(dst)
fs.ensureDirSync(dstBin)

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