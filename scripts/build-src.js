#!/usr/bin/env node

const path = require('path')
const glob = require('glob')
const execa = require('execa')
const fs = require('fs-extra')

const src = path.resolve(__dirname, '../src')
const dst = path.resolve(__dirname, '../genet_modules/src')
const webpack = path.resolve(__dirname, '../node_modules/.bin/webpack-cli')
const lessc = path.resolve(__dirname, '../node_modules/.bin/lessc')
fs.ensureDirSync(dst)
fs.ensureDirSync(path.resolve(dst, 'asset'))

const lessFiles = glob.sync(path.resolve(src, 'asset/*.main.less'))
const assets = glob.sync(path.resolve(src, 'asset/*.htm'))

execa(webpack, [], {
  stdio: 'inherit'
})
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