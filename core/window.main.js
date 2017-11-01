import { ipcRenderer, remote } from 'electron'
import ThemeLoader from './theme-loader'

export default async function (argv) {
  const loader = new ThemeLoader(`${__dirname}/theme.less`)
  await loader.load(`${__dirname}/window.less`, document.head)
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
