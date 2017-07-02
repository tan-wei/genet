import DissectorComponent from './components/dissector'
import Plugin from './plugin'
import Session from './session'
import StreamDissectorComponent from './components/stream-dissector'

Plugin.listPlugins()
.then((list) => {
  const tasks = []
  for (const plugin of list) {
    for (const comp of plugin.compList) {
      if (comp.type === 'dissector') {
        const dissector =
          new DissectorComponent(plugin.rootDir, plugin.pkg, comp)
        tasks.push(dissector.load())
      } else if (comp.type === 'stream-dissector') {
        const dissector =
          new StreamDissectorComponent(plugin.rootDir, plugin.pkg, comp)
        tasks.push(dissector.load())
      }
    }
  }
  return Promise.all(tasks)
})
.then(() => Session.runSampleBenchmark())
.then((result) => {
  const fps = result[0].frames / (result[0].duration / 1000000000)
  console.log(`${fps} frames/sec`) // eslint-disable-line no-console

  // eslint-disable-next-line no-process-exit
  process.exit(0)
})
.catch(() => {

  // eslint-disable-next-line no-process-exit
  process.exit(-1)
})
