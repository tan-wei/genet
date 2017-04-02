import { ipcRenderer, remote } from 'electron'
import deplug from './deplug'

export default async function (argv) {
  const { Parcel, } = await deplug(argv)
  await Parcel.loadComponents('window')
  ipcRenderer.send('window-deplug-loaded', remote.getCurrentWindow().id)
}
