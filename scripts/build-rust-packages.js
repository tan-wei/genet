#!/usr/bin/env node

const path = require('path')
const glob = require('glob')
const execa = require('execa')

const dirs = glob.sync(path.join(__dirname, '../package/*/crates/*/Cargo.toml'))
  .map((toml) => path.dirname(toml))

async function exec() {
  const mode = process.env.CI ? '--release' : ''
  for (const dir of dirs) {
    await execa.shell(`cargo build ${mode}`, {
      cwd: dir,
      stdio: 'inherit'
    })
  }
}

exec()
