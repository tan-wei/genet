import builtin from 'builtin-modules'
import config from './config'
import log from 'electron-log'
import path from 'path'
import { rollup } from 'rollup'
import vm from 'vm'

export default async function roll (file, rootDir, extern = []) {
  const deplugExtern = ['deplug', 'electron']
  const globalExtern = Object.keys(config.deplug.dependencies)
  const bundle = await rollup({
    entry: file,
    external: extern.concat(builtin, deplugExtern, globalExtern),
    acorn: { ecmaVersion: 8 },
    onwarn: (err) => {
      log.warn(err)
    },
  })
  const result = bundle.generate({ format: 'cjs' })
  return (module) => {
    const options = {
      filename: file,
      displayErrors: true,
    }
    const code =
      `(function(module, require, __filename, __dirname){ ${result.code} })`
    const func = vm.runInThisContext(code, options)
    return func(module, require, file, path.dirname(file))
  }
}
