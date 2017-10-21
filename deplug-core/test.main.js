import DissectorComponent from './components/dissector'
import Plugin from './plugin'
import Session from './session'
import assert from 'assert'
import objpath from 'object-path'

Plugin.listPlugins()
.then((list) => {
  const tasks = []
  for (const plugin of list) {
    for (const comp of plugin.compList) {
      if (comp._type === 'core:dissector') {
        const dissector =
          new DissectorComponent(plugin.rootDir, plugin.pkg, comp)
        tasks.push(dissector.load())
      }
    }
  }
  return Promise.all(tasks)
})
.then(() => Session.runSampleAnalysis())
.then((list) => {
  for (const frame of list) {
    const { sourceId } = frame
    const sample = Session.samples[sourceId]
    if (sample.assert) {
      for (const line of sample.assert) {
        for (const key in line.properties) {
          const value = objpath.get(frame.layer(line.layer), key)
          assert.deepEqual(value, line.properties[key])
        }
      }
    }
  }

  // eslint-disable-next-line no-process-exit
  process.exit(0)
})
.catch(() => {

  // eslint-disable-next-line no-process-exit
  process.exit(-1)
})
