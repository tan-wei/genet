import { ipcRenderer, remote } from 'electron'
import deplug from './deplug'
import jquery from 'jquery'

export default async function (argv) {
  const { Parcel, } = await deplug(argv)
  await Parcel.loadComponents('window')
  await new Promise((res) => {
    jquery(res)
  })
  ipcRenderer.send('window-deplug-loaded', remote.getCurrentWindow().id)
}
