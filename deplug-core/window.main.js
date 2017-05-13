import { ipcRenderer, remote } from 'electron'
import { Pcap } from 'plugkit'
import deplug from './deplug'
import fs from 'fs'
import path from 'path'

const { dialog } = remote
export default async function (argv) {
  try {
    const { File, Plugin, GlobalChannel } = await deplug(argv)
    await Plugin.loadComponents('theme')
    await Plugin.loadComponents('file')
    await Plugin.loadComponents('window')
    await Plugin.loadComponents('menu')
    await Plugin.loadComponents('tab')
    GlobalChannel.emit('core:window:loaded')
    process.nextTick(() => {
      if (process.platform === 'darwin' && !Pcap.permission) {
        GlobalChannel.emit('core:tab:open', 'Helper')
      } else {
        GlobalChannel.emit('core:tab:open', 'Pcap')
      }
    })

    GlobalChannel.on('core:file:import', () => {
      const filters = []
      for (const imp of File.importers()) {
        filters.push({
          name: imp.name,
          extensions: imp.extensions,
        })
      }
      const files = dialog.showOpenDialog(
        remote.getCurrentWindow(), { filters })
      if (files) {
        for (const file of files) {
          const ext = path.extname(file).substr(1)
          for (const imp of File.importers(ext)) {
            const Handler = imp.handler
            const handler = new Handler()
            handler.loadFrames(fs.createReadStream(file)).then((result) => {
              GlobalChannel.emit('core:file:frame-loaded', result)
            })
          }
        }
      }
    })

  } catch (err) {
    remote.getCurrentWindow().openDevTools()
    // eslint-disable-next-line no-console
    console.error(err)
  } finally {
    ipcRenderer.send('window-deplug-loaded', remote.getCurrentWindow().id)
  }
}

document.addEventListener('dragover', (event) => {
  event.preventDefault()
  return false
}, false)

document.addEventListener('drop', (event) => {
  event.preventDefault()
  return false
}, false)
