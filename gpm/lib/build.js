const objpath = require('object-path')
const os = require('os')
const execa = require('execa')
const glob = require('glob')
const path = require('path')
const fs = require('fs-extra')
const toml = require('toml')
const { intersect } = require('semver-intersect')

const sep = (process.platform === 'win32')
  ? ';'
  : ':'
async function cargo (dir, version, target) {
  const cargoFiles = glob.sync(path.join(dir, '*/**/Cargo.toml'))
  if (cargoFiles.length === 0) {
    return
  }

  console.log(`Building package in ${target} mode ...`)
  const cargoPath = path.join(os.homedir(), '.cargo', 'bin')
  const envpath =
    `${process.env.PATH || ''}${sep}${process.env.CARGOPATH || cargoPath}`
  try {
    await execa.shell('cargo -V',
      { env: Object.assign(process.env, { PATH: envpath }) })
  } catch (err) {
    throw new Error(`
  Cargo command is not found:
  
  You need a rust toolchain to install native packages.
  
  Visit https://www.rustup.rs/ for installation details.`)
  }
  for (const cfile of cargoFiles) {
    const config = toml.parse(fs.readFileSync(cfile, 'utf8'))
    const dep = objpath.get(config, 'dependencies.genet-sdk', '').toString()
    try {
      intersect(dep, version)
    } catch (err) {
      console.warn(
        `This package uses an incompatible version of genet-sdk: ${dep}`)
    }
  }

  const flags = process.env.RUSTFLAGS || '-C target-cpu=native'
  const args = target === 'debug'
    ? []
    : ['--release']
  const proc = execa.shell(['cargo', 'build'].concat(args).join(' '), {
    cwd: dir,
    env: Object.assign(process.env, {
      RUSTFLAGS: flags,
      PATH: envpath,
    }),
    stdio: 'inherit',
  })
  await proc
}

async function npm (dir) {
  console.log('Reading package.json ...\n')
  const pkg = fs.readJson(path.join(dir, 'package.json'))
  if (!objpath.has(pkg, 'dependencies') && !objpath.has(pkg, 'scripts')) {
    return
  }
  const envpath = (process.env.PATH || '') +
    process.env.NPMPATH
    ? `${sep}${process.env.NPMPATH}`
    : ''
  try {
    await execa.shell('npm version', { env: envpath })
  } catch (err) {
    throw new Error(`
  NPM command is not found:
  
  You need NPM (Node Package Manager) to resolve dependencies for the package.
  
  Visit https://www.npmjs.com/get-npm for installation details.`)
  }
  const proc = execa.shell('npm install --production', {
    cwd: dir,
    stdio: 'inherit',
    env: envpath,
  })
  return proc
}

module.exports = {
  cargo,
  npm,
}
