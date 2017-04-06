import { ipcRenderer, remote } from 'electron'
import deplug from './deplug'
import jquery from 'jquery'

export default async function (argv) {
  try {
    const { Parcel, Channel, } = await deplug(argv)
    await Parcel.loadComponents('window')
    await Parcel.loadComponents('tab')
    await new Promise((res) => {
      jquery(res)
    })
    Channel.emit('core:window-loaded')
    process.nextTick(() => {
      Channel.emit('core:create-tab', 'Preferences')
      Channel.emit('core:create-tab', 'Preferences')
      Channel.emit('core:create-tab', 'Preferences')
      Channel.emit('core:create-tab', 'Preferences')
      Channel.emit('core:create-tab', 'Preferences')
    })

  } catch (err) {
    remote.getCurrentWindow().openDevTools()
    throw err
  } finally {
    ipcRenderer.send('window-deplug-loaded', remote.getCurrentWindow().id)
  }
}
