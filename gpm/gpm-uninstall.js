const program = require('commander')
const fs = require('fs-extra')
const listPackages = require('./lib/list')
const { version, builtinPackagePath, userPackagePath } = require('./lib/env')
program
  .version(version)
  .option('-d, --dst <path>', 'specify installation directory')
  .usage('[packages...]')
  .parse(process.argv)

const dst = program.dst || userPackagePath
const pkgs = listPackages({
  builtinDirs: builtinPackagePath
    ? [builtinPackagePath]
    : [],
  userDirs: [dst],
})
if (program.args.length === 0) {
  program.help()
}

pkgs.then((list) => {
  for (const id of program.args) {
    const pkg = list.find((item) => (item.id === id ||
      item.metadata.name === id) &&
      item.source !== ':builtin:')
    if (!pkg) {
      console.error(`package not found: ${id}`)
      process.exit(1)
    }
    console.log(`removing ${pkg.metadata.name} ...`)
    fs.removeSync(pkg.dir)
  }
  process.exit(0)
}).catch((err) => {
  console.error(`${err.message}`)
  process.exit(1)
})
