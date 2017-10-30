import { ipcRenderer, remote } from 'electron'

export default async function (argv) {
  ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
}

document.addEventListener('dragover', (event) => {
  event.preventDefault()
  return false
}, false)

document.addEventListener('drop', (event) => {
  event.preventDefault()
  return false
}, false)
