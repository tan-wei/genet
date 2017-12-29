#!/usr/bin/env node

const { spawnSync, spawn } = require('child_process')
const chokidar = require('chokidar')
const touch = require('touch')
const debounce = require('lodash.debounce')
const watchMode = (process.argv[2] === '--watch')

const reloadFile = '.reload'
const negatron = (process.platform === 'win32')
                  ? 'node_modules\\.bin\\negatron.cmd'
                  : 'node_modules/.bin/negatron'

let building = false

function run(cmd, ...args) {
  return spawnSync(cmd, args, {
    stdio: 'inherit'
  })
}

function build() {
  if (building) return
  building = true
  run('node', 'scripts/run-as-node.js', negatron, 'scripts/generate-version-file.js')
  run('node', 'scripts/build-plugkit.js')
  run('node', 'scripts/build-packages.js')
  run('node', 'scripts/build-deplug-core.js')
  touch.sync(reloadFile)
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

  spawn(negatron, [
      '--enable-logging',
      `--deplug-dev-watch-file=./${reloadFile}`,
      '.'
    ])
    .on('exit', process.exit)
}
