#!/usr/bin/env node

const path = require('path')
const glob = require('glob')
const execa = require('execa')

const dirs = glob.sync(path.join(__dirname, '../package/*/crates/*/Cargo.toml'))
  .map((toml) => path.dirname(toml))

async function exec() {
  for (const dir of dirs) {
    const proc = execa.shell('cargo build --release --color=always', {
      cwd: dir
    })
    proc.stdout.pipe(process.stdout)
    proc.stderr.pipe(process.stderr)
    await proc
  }
}

exec()
