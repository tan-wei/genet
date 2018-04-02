#!/usr/bin/env node

const mkpath = require('mkpath')
const path = require('path')
const glob = require('glob')
const execa = require('execa')
const fs = require('fs')
const os = require('os')
const jsonfile = require('jsonfile')

const src = path.resolve(__dirname, '../plugkit')
const rustSrc = path.resolve(src, 'plugkit.rs')
const dst = path.resolve(__dirname, '../deplug-modules/plugkit')
const dstBin = path.resolve(__dirname, '../deplug-modules/bin/plugkit')
const cache = path.resolve(dst, '.last-updated')
const scriptFiles = glob.sync(path.resolve(src, '*.{js,json}'))
const embeddedFiles = glob.sync(path.resolve(src, 'js/*.js'))
const env = require('./plugkit-npm-env')

mkpath.sync(dst)
mkpath.sync(dstBin)
mkpath.sync(path.resolve(dst, 'build/Release'))
mkpath.sync(path.resolve(dst, 'include/plugkit'))

execa.sync(path.resolve(__dirname, 'text-to-cpp.js'),
embeddedFiles.concat(path.resolve(src, 'src/embedded_files.hpp')))

async function exec() {
  await execa.shell('cargo build --release', {
    cwd: rustSrc,
    stdio: 'inherit'
  })

  await execa('node-gyp', ['configure'], { env, cwd: src, stdio: 'inherit' })
  await execa('node-gyp', ['build'], { env, cwd: src, stdio: 'inherit' })

  const binaryFiles = glob.sync(path.resolve(src, 'build/Release/*.{node,lib}'))
  for (const file of binaryFiles) {
    fs.createReadStream(file)
      .pipe(fs.createWriteStream(
        path.resolve(dstBin, path.basename(file))))
    fs.createReadStream(file)
      .pipe(fs.createWriteStream(
      path.resolve(dst, 'build/Release', path.basename(file))))
  }

  for (const file of scriptFiles) {
  fs.createReadStream(file)
    .pipe(fs.createWriteStream(
      path.resolve(dst, path.basename(file))))
  }

  const headerFiles = glob.sync(path.resolve(src, 'include/plugkit/*.h'))
  for (const file of headerFiles) {
  fs.createReadStream(file)
    .pipe(fs.createWriteStream(
      path.resolve(dst, 'include/plugkit', path.basename(file))))
  }
}

exec()