import { ipcRenderer, remote } from 'electron'
import deplug from './deplug'

export default async function (argv) {
  await deplug(argv)
  ipcRenderer.send('window-deplug-loaded', remote.getCurrentWindow().id)
  document.write('Hey!')
}
