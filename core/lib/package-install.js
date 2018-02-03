import { EventEmitter } from 'events'
import axios from 'axios'
import execa from 'execa'
import glob from 'glob'
import jsonfile from 'jsonfile'
import mkpath from 'mkpath'
import objpath from 'object-path'
import path from 'path'
import promisify from 'es6-promisify'
import tar from 'tar'
import zlib from 'zlib'

const promiseReadJsonFile = promisify(jsonfile.readFile)
const promiseGlob = promisify(glob)
const promiseMkpath = promisify(mkpath)
const fields = Symbol('fields')
export default class PackageInstaller extends EventEmitter {
  constructor () {
    super()
    this[fields] = {
      rustpath: '',
      rustflags: '',
    }
  }

  get rustpath () {
    return this[fields].rustpath
  }

  set rustpath (rpath) {
    this[fields].rustpath = rpath
  }

  get rustflags () {
    return this[fields].rustflags
  }

  set rustflags (flags) {
    this[fields].rustflags = flags
  }

  async install (dir, url) {
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
    await new Promise((res, rej) => {
      response.data.pipe(gunzip)
        .pipe(extractor)
        .on('error', rej)
        .on('finish', res)
    })
    await this.build(dir)
    await this.npm(dir)
    this.emit('output', 'Done\n')
  }

  async build (dir) {
    this.emit('output', 'Building package ...\n')
    try {
      await execa.shell('cargo -V')
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
      const flags = `${process.env.RUSTFLAGS || ''} ${this.rustflags}`
      const envpath = `${process.env.PATH || ''};${this.rustpath}`
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
    const pkg = promiseReadJsonFile(path.join(dir, 'package.json'))
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
