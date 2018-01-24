#!/usr/bin/env node

const fs = require('fs')
const path = require('path')
const glob = require('glob')
const execa = require('execa')
const mkpath = require('mkpath')

const dirs = glob.sync(path.join(__dirname, '../package/**/Cargo.toml'))
  .map((toml) => path.dirname(toml))

async function exec() {
  for (const dir of dirs) {
    const proc = execa.shell('cargo build --release --color=always', {
      cwd: dir
    })
    proc.stdout.pipe(process.stdout)
    proc.stderr.pipe(process.stderr)
    await proc
    const libs = glob.sync(
      path.join(__dirname, '../package/**/target/release/*.{dylib,so,dll}'))
    if (libs.length > 0) {
      const [lib] = libs
      const dst = path.join(dir, 'build/Release')
      mkpath.sync(dst)
      fs.createReadStream(lib)
        .pipe(fs.createWriteStream(path.join(dst, 'main.node')))
    }
  }
}

exec()
