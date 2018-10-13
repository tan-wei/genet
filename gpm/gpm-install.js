const program = require('commander')
const chokidar = require('chokidar')
const isLocal = require('is-local-path')
const semver = require('semver')
const fs = require('fs-extra')
const os = require('os')
const path = require('path')
const fetch = require('./lib/fetch')
const verify = require('./lib/verify')
const install = require('./lib/install')
const { cargo, npm } = require('./lib/build')
const {
  version, genetAbiVersion, genetVersion, genetTarget,
  userPackagePath,
} = require('./lib/env')
program
  .version(version)
  .option('-s, --sdk <ver>', 'specify SDK version',
    (val) => semver.valid(semver.coerce(val)) || false)
  .option('-d, --dst <path>', 'specify installation directory')
  .option('-w, --watch', 'automatically install updated local packages')
  .option('--debug', 'debug build')
  .option('--release', 'release build')
  .usage('[packages...]')
  .parse(process.argv)

if (program.args.length === 0) {
  program.help()
}

const sdkVersion = program.sdk || genetAbiVersion
if (!sdkVersion) {
  console.error('Failed to detect genet SDK version.')
  console.error('Launch genet app once or ' +
    'use --sdk option to specify SDK verison explicitly.')
  process.exit(1)
}

const debugBuild = program.debug || genetTarget === 'debug'
const releaseBuild = program.release || genetTarget === 'release'
if (!debugBuild && !releaseBuild) {
  console.info('Failed to detect genet target (debug/release).')
  console.info('Attempt to build both targets.')
}

const dst = program.dst || userPackagePath
async function run (source) {
  const dir = await fetch(source)
  await verify(dir, genetVersion)
  await npm(dir)
  if (debugBuild || (!debugBuild && !releaseBuild)) {
    await cargo(dir, sdkVersion, 'debug')
  }
  if (releaseBuild || (!debugBuild && !releaseBuild)) {
    await cargo(dir, sdkVersion, 'release')
  }
  await install(source, dst, dir)
}

async function runall (list) {
  return Promise.all(list.map((pkg) => run(pkg)))
    .then(() => {
      console.log('✔ installed')
      return Promise.resolve()
    })
}

function trigger () {
  runall(program.args)
    .then(() => {
      fs.outputFileSync(
        path.join(os.homedir(), '.genet', '.reload'), `${Date.now()}`)
    })
    .catch((err) => {
      console.error(`${err.message}`)
    })
}

if (program.watch) {
  chokidar
    .watch(program.args.filter((pkg) => isLocal(pkg)), {
      ignoreInitial: true,
      ignored: '**/target/**/*',
      atomic: 1000,
    })
    .on('all', trigger)
  trigger()
} else {
  runall(program.args)
    .then(() => {
      process.exit(0)
    })
    .catch((err) => {
      console.error(`${err.message}`)
      process.exit(1)
    })
}
