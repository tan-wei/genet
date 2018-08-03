#!/usr/bin/env node

const { spawnSync } = require('child_process')

function run(cmd, ...args) {
  return spawnSync(cmd, args, {
    stdio: 'inherit'
  })
}

run('node', 'scripts/build-src.js')
run('node', 'scripts/build-lib.js')
run('node', 'scripts/build-rust-packages.js')

if (process.env.NODE_ENV === 'production') {
  run('node', 'scripts/generate-license-json.js')
}
