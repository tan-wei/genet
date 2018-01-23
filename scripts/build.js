#!/usr/bin/env node

const { spawnSync, spawn } = require('child_process')
const chokidar = require('chokidar')
const touch = require('touch')
const debounce = require('lodash.debounce')
const path = require('path')
const notifier = require('node-notifier')
const watchMode = (process.argv[2] === '--watch')
const { negatronBin } = require('./negatron')
const runAsNode = require('./run-as-node')

const reloadFile = '.reload'
let building = false

function run(cmd, ...args) {
  return spawnSync(cmd, args, {
    stdio: 'inherit'
  })
}

function build() {
  if (building) return
  building = true
  runAsNode('scripts/generate-version-file.js')
  run('node', 'scripts/build-plugkit.js')
  run('node', 'scripts/build-packages.js')
  run('node', 'scripts/build-rust-packages.js')
  run('node', 'scripts/build-deplug-core.js')
  touch.sync(reloadFile)
  if (watchMode) {
    notifier.notify({
      title: 'Deplug Build Service',
      message: 'Build Finished!\nReloading Window...',
      icon: path.join(__dirname, '..', 'images', 'deplug.png')
    })
  }
  setTimeout(() => {
    building = false
  }, 1000)
}

build()

const triggerBuild = debounce(() => {
  console.log('updated...')
  build()
}, 500)

if (watchMode) {
  chokidar.watch('.', {
    ignored: /(^|[\/\\])\../,
    ignoreInitial: true
  }).on('all', () => {
    if (!building) {
      triggerBuild()
    }
  })

  spawn(negatronBin, [
      '--enable-logging',
      `--deplug-dev-watch-file=./${reloadFile}`,
      '.'
    ], {
      stdio: 'inherit'
    })
    .on('exit', process.exit)
}
