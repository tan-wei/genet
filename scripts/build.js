#!/usr/bin/env node

const { spawnSync } = require('child_process')
const runAsNode = require('./run-as-node')

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
  run('node', 'scripts/build-src.js')
  run('node', 'scripts/build-lib.js')
  run('node', 'scripts/build-rust-packages.js')
}

build()
