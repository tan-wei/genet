#!/usr/bin/env node

const path = require('path')
const execa = require('execa')

const src = path.resolve(__dirname, '../dyr-core')
const rustSrc = path.resolve(src, 'dyr_kernel')

async function exec() {
  await execa.shell('cargo build --release', {
    cwd: rustSrc,
    stdio: 'inherit'
  })
}

exec()