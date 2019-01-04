#!/usr/bin/env node

const path = require('path')
const glob = require('glob')
const execa = require('execa')

const mode = process.env.NODE_ENV === 'production'
  ? ['--release']
  : []

const dirs = glob.sync(path.join(__dirname, '../package/*/Cargo.toml'))
  .map((toml) => path.dirname(toml))

async function exec() {
  for (const dir of dirs) {
    console.log(dir)
    await execa('cargo', ['build'].concat(mode), {
      cwd: dir,
      stdio: 'inherit'
    })
  }
}

exec()
