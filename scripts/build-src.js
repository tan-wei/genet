#!/usr/bin/env node

const mkpath = require('mkpath')
const path = require('path')
const glob = require('glob')
const execa = require('execa')
const fs = require('fs')

const src = path.resolve(__dirname, '../src')
const dst = path.resolve(__dirname, '../genet_modules/src')
const webpack = path.resolve(__dirname, '../node_modules/.bin/webpack')
const lessc = path.resolve(__dirname, '../node_modules/.bin/lessc')
mkpath.sync(dst)
mkpath.sync(path.resolve(dst, 'asset'))

const jsFiles = glob.sync(path.resolve(src, '*.main.js'))
const lessFiles = glob.sync(path.resolve(src, 'asset/*.main.less'))
const assets = glob.sync(path.resolve(src, 'asset/*.htm'))

for (const file of jsFiles) {
  execa(webpack, [file, path.resolve(dst, path.basename(file))])
    .stdout.pipe(process.stdout)
}
for (const file of lessFiles) {
  const css = 
    path.resolve(path.resolve(dst, 'asset'), path.basename(file, '.less')) + '.css'
  execa(lessc, [file, css])
    .stdout.pipe(process.stdout)
}
for (const file of assets) {
  fs.createReadStream(file)
    .pipe(fs.createWriteStream(
      path.resolve(dst, 'asset', path.basename(file))))
}