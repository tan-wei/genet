#!/usr/bin/env node

const mkpath = require('mkpath')
const path = require('path')
const glob = require('glob')
const execa = require('execa')
const fs = require('fs')
const os = require('os')
const jsonfile = require('jsonfile')
const lastmod = require('./lastmod')

const src = path.resolve(__dirname, '../plugkit')
const dst = path.resolve(__dirname, '../deplug-modules/plugkit')
const dstBin = path.resolve(__dirname, '../deplug-modules/bin/plugkit')
const cache = path.resolve(dst, '.last-updated')
const version = jsonfile.readFileSync(
  path.join(__dirname, '../package.json')).devDependencies.negatron
const scriptFiles = glob.sync(path.resolve(src, '*.{js,json}'))
const embeddedFiles = glob.sync(path.resolve(src, 'js/*.js'))
const gperf = (process.platform === 'win32')
  ? path.resolve(__dirname, '../node_modules/.bin/gperf')
  : '/usr/bin/gperf'

const env = Object.assign(process.env, {
  npm_config_target: version,
  npm_config_devdir: path.resolve(os.homedir(), '.electron-gyp'),
  npm_config_disturl: 'https://atom.io/download/electron',
  npm_config_arch: process.arch,
  npm_config_target_arch: process.arch,
  npm_config_runtime: 'electron',
  npm_config_build_from_source: 'true',
  jobs: '4'
})

mkpath.sync(dst)
mkpath.sync(dstBin)
mkpath.sync(path.resolve(dst, 'include/plugkit'))

let lastUpdated = null
try {
  lastUpdated = Number.parseInt(fs.readFileSync(cache, 'utf8'))
} catch (err) {
  lastUpdated = 0
}

const srcLastUpdated = lastmod(src).getTime()
if (srcLastUpdated > lastUpdated) {
  execa.sync(path.resolve(__dirname, 'text-to-cpp.js'),
    embeddedFiles.concat(path.resolve(src, 'src/embedded_files.hpp')))

  execa.sync(gperf, [
    '-LANSI-C',
    path.resolve(src, 'src/token.keys'),
    '-G',
    `--output-file=${path.resolve(src, 'src/token_hash.h')}`
  ])

  execa('node-gyp', ['configure'], { env, cwd: src }).then(() => {
  	const proc = execa('node-gyp', ['build'], { env, cwd: src })
    proc.stdout.pipe(process.stdout)
    proc.stderr.pipe(process.stderr)
    return proc
  }).then(() => {
    const binaryFiles = glob.sync(path.resolve(src, 'build/Release/*.{node,lib}'))
    for (const file of binaryFiles) {
      fs.createReadStream(file)
        .pipe(fs.createWriteStream(
          path.resolve(dstBin, path.basename(file))))
    }
  })

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

process.on('exit', () => {
  fs.writeFileSync(cache, `${lastmod(src).getTime()}`)
})
