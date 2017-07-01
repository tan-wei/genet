import { ipcRenderer, remote } from 'electron'
import { Pcap } from 'plugkit'
import deplug from './deplug'

const { dialog } = remote
export default async function (profile, argv) {
  try {
    const { File, PluginLoader, GlobalChannel } = await deplug(profile, argv)
    await PluginLoader.loadComponents('theme')
    await PluginLoader.loadComponents('file')
    await PluginLoader.loadComponents('window')
    await PluginLoader.loadComponents('menu')
    await PluginLoader.loadComponents('tab')
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
        GlobalChannel.emit('core:tab:open', 'Pcap', { files })
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
