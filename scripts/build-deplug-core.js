#!/usr/bin/env node

const mkpath = require('mkpath')
const path = require('path')
const glob = require('glob')
const execa = require('execa')
const fs = require('fs')
const lastmod = require('./lastmod')

const src = path.resolve(__dirname, '../core')
const dst = path.resolve(__dirname, '../deplug-modules/core')
const cache = path.resolve(dst, '.last-updated')
const webpack = path.resolve(__dirname, '../node_modules/.bin/webpack')
mkpath.sync(dst)
mkpath.sync(path.resolve(dst, 'asset'))

const files = glob.sync(path.resolve(src, '*.main.js'))
const assets = glob.sync(path.resolve(src, 'asset/*.*'))

let lastUpdated = null
try {
  lastUpdated = Number.parseInt(fs.readFileSync(cache, 'utf8'))
} catch (err) {
  lastUpdated = 0
}

const srcLastUpdated = lastmod(src).getTime()
if (srcLastUpdated > lastUpdated) {
  for (const file of files) {
    execa(webpack, [file, path.resolve(dst, path.basename(file))])
      .stdout.pipe(process.stdout)
  }
  for (const file of assets) {
    fs.createReadStream(file)
      .pipe(fs.createWriteStream(
        path.resolve(dst, 'asset', path.basename(file))))
  }
}

process.on('exit', () => {
  fs.writeFileSync(cache, `${lastmod(src).getTime()}`)
})
