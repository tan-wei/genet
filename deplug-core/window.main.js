import { ipcRenderer, remote } from 'electron'
import deplug from './deplug'
import jquery from 'jquery'

export default async function (argv) {
  try {
    const { Plugin, Channel, } = await deplug(argv)
    await Plugin.loadComponents('window')
    await Plugin.loadComponents('tab')
    await new Promise((res) => {
      jquery(res)
    })
    Channel.emit('core:window:loaded')
    process.nextTick(() => {
      Channel.emit('core:tab:open', 'Pcap')
      Channel.emit('core:tab:open', 'Pcap')
      Channel.emit('core:tab:open', 'Pcap')
    })
    setInterval(() => {
      Channel.emit('core:tab:set-name', 1, `PREF ${Math.random()}`)
      Channel.emit('core:tab:close', 2, `PREF ${Math.random()}`)
    }, 5000)

  } catch (err) {
    remote.getCurrentWindow().openDevTools()
    // eslint-disable-next-line no-console
    console.error(err)
  } finally {
    ipcRenderer.send('window-deplug-loaded', remote.getCurrentWindow().id)
  }
}
