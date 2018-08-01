#!/usr/bin/env node

const { spawnSync, spawn } = require('child_process')
const { ensureFileSync } = require('fs-extra')
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
  run('node', 'scripts/build-src.js')
  run('node', 'scripts/build-lib.js')
  run('node', 'scripts/build-rust-packages.js')
  ensureFileSync(reloadFile)
  setTimeout(() => {
    building = false
  }, 1000)
}

build()
