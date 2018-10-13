const program = require('commander')
const { table } = require('table')
const listPackages = require('./lib/list')
const { version, builtinPackagePath, userPackagePath } = require('./lib/env')
program
  .version(version)
  .option('-j, --json', 'use JSON format')
  .option('-d, --dst <path>', 'specify installation directory')
  .parse(process.argv)

const dst = program.dst || userPackagePath

const pkgs = listPackages({
  builtinDirs: builtinPackagePath
    ? [builtinPackagePath]
    : [],
  userDirs: [dst],
})
pkgs.then((list) => {
  if (program.json) {
    console.log(JSON.stringify(list))
  } else if (list.length === 0) {
    console.log('There are no installed packages.')
  } else {
    console.log(table(list.map((pkg) => [
      pkg.metadata.name,
      pkg.metadata.version,
      pkg.source,
      pkg.metadata.description
    ])))
  }
  process.exit(0)
})
  .catch((err) => {
    console.error(`${err.message}`)
    process.exit(1)
  })
