import { ipcRenderer, remote } from 'electron'
import deplug from './deplug'
import jquery from 'jquery'

export default async function (argv) {
  try {
    const { Parcel, } = await deplug(argv)
    await Parcel.loadComponents('window')
    await Parcel.loadComponents('tab')
    await new Promise((res) => {
      jquery(res)
    })
  } catch (err) {
    remote.getCurrentWindow().openDevTools()
    throw err
  } finally {
    ipcRenderer.send('window-deplug-loaded', remote.getCurrentWindow().id)
  }
}
