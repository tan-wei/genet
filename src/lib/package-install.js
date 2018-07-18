import { EventEmitter } from 'events'
import axios from 'axios'
import execa from 'execa'
import fs from 'fs-extra'
import glob from 'glob'
import mkpath from 'mkpath'
import objpath from 'object-path'
import os from 'os'
import path from 'path'
import { promisify } from 'util'
import rimraf from 'rimraf'
import tar from 'tar'
import tmp from 'tmp-promise'
import zlib from 'zlib'

const promiseGlob = promisify(glob)
const promiseMkpath = promisify(mkpath)
const promiseRmdir = promisify(rimraf)
const promiseRename = promisify(fs.rename)
const fields = Symbol('fields')
export default class PackageInstaller extends EventEmitter {
  constructor () {
    super()
    this[fields] = { rustpath: '' }
  }

  get rustpath () {
    return this[fields].rustpath
  }

  set rustpath (rpath) {
    this[fields].rustpath = rpath
  }

  async install (dir, url) {
    try {
      const tmpdir = await tmp.dir()
      await this.download(tmpdir.path, url)
      await this.build(tmpdir.path)
      await this.npm(tmpdir.path)
      await promiseRmdir(dir).catch((err) => this.emit('output', `${err}\n`))
      await promiseRename(tmpdir.path, dir)
      this.emit('output', 'Done\n')
    } catch (err) {
      this.emit('output', 'Failed\n')
      await promiseRmdir(dir)
      throw err
    }
  }

  async download (dir, url) {
    this.emit('output', `Downloading ${url} ...\n`)
    const response = await axios({
      method: 'get',
      url,
      responseType: 'stream',
    })
    const gunzip = zlib.createGunzip()
    const extractor = new tar.Unpack({
      cwd: dir,
      strip: 1,
    })
    await promiseMkpath(dir)
    return new Promise((res, rej) => {
      response.data.pipe(gunzip)
        .pipe(extractor)
        .on('error', rej)
        .on('finish', res)
    })
  }

  async build (dir) {
    this.emit('output', 'Building package ...\n')
    const sep = (process.platform === 'win32')
      ? ';'
      : ':'
    const rustdir = path.join(os.homedir(), '.cargo', 'bin')
    const envpath =
      `${process.env.PATH || ''}${sep}${this.rustpath || rustdir}`
    try {
      await execa.shell('cargo -V',
        { env: Object.assign(process.env, { PATH: envpath }) })
    } catch (err) {
      throw new Error(`
Cargo command is not found:

You need a rust toolchain to install native packages.

Visit https://www.rustup.rs/ for installation details.
      `)
    }
    const cargoFiles =
      await promiseGlob(path.join(dir, 'crates/*/Cargo.toml'))
    const cargoDirs = cargoFiles.map((toml) => path.dirname(toml))
    for (const cdir of cargoDirs) {
      const flags = process.env.RUSTFLAGS || '-C target-cpu=native'
      const proc = execa.shell(
        'cargo build -v --release', {
          cwd: cdir,
          env: Object.assign(process.env, {
            RUSTFLAGS: flags,
            PATH: envpath,
          }),
        })
      proc.stdout.on('data', (chunk) => {
        this.emit('output', chunk.toString('utf8'))
      })
      proc.stderr.on('data', (chunk) => {
        this.emit('output', chunk.toString('utf8'))
      })
      // eslint-disable-next-line no-await-in-loop
      await proc
    }
  }

  async npm (dir) {
    this.emit('output', 'Reading package.json ...\n')
    const pkg = fs.readJson(path.join(dir, 'package.json'))
    if (!objpath.has(pkg, 'dependencies') && !objpath.has(pkg, 'scripts')) {
      return
    }
    try {
      await execa.shell('npm -V')
    } catch (err) {
      throw new Error(`
NPM command is not found:

You need NPM (Node Package Manager) to resolve dependencies for the package.

Visit https://www.npmjs.com/get-npm for installation details.
      `)
    }
    const proc = execa.shell(
      'npm install --production', { cwd: dir })
    proc.stdout.on('data', (chunk) => {
      this.emit('output', chunk.toString('utf8'))
    })
    proc.stderr.on('data', (chunk) => {
      this.emit('output', chunk.toString('utf8'))
    })
    return proc
  }
}
