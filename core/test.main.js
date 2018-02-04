/* eslint no-console: "off", no-process-exit: "off" */

import ComponentFactory from './lib/component-factory'
import Config from './lib/config'
import Session from './lib/session'
import env from './lib/env'
import glob from 'glob'
import jsonfile from 'jsonfile'
import path from 'path'

console.log('')
console.log('Running built-in dissector tests...')
console.log('')

const config = new Config('default', 'config')
const session = new Session(config)
global.deplug = { session }

const builtinPluginPattern =
  path.join(env.builtinPackagePath, '/**/package.json')
const packages = glob.sync(builtinPluginPattern)
  .map((file) => ({
    list: jsonfile.readFileSync(file).deplug.components,
    dir: path.dirname(file),
  }))

const task = []
for (const pkg of packages) {
  for (const comp of pkg.list) {
    if (comp.type.startsWith('core:dissector:') ||
        comp.type.startsWith('core:token') ||
        comp.type.startsWith('core:file:')) {
      task.push(ComponentFactory.create(comp, pkg.dir).load())
    }
  }
}

Promise.all(task).then(() => {
  session.runSampleTestingAll().then((reports) => {
    let okTotal = 0
    let ngTotal = 0
    for (const report of reports) {
      let ok = 0
      let ng = 0
      for (let index = 0; index < report.results.length; index += 1) {
        const result = report.results[index]
        for (const filter of result.assert) {
          if (filter.match) {
            ok += 1
          } else {
            console.warn(` ! ERROR: Frame#${index}: condition is` +
              ` not met: '${filter.filter}'`)
            ng += 1
          }
        }
      }
      console.log(` * pcap:    ${report.sample.pcap}`)
      console.log(` * assert:  ${report.sample.assert}`)
      console.log(` * ok: ${ok} ng: ${ng}`)
      console.log('')
      okTotal += ok
      ngTotal += ng
    }
    console.log(`** ok: ${okTotal} ng: ${ngTotal}`)
    if (ngTotal === 0) {
      process.exit(0)
    } else {
      process.exit(1)
    }
  })
})
