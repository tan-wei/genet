import Content from './lib/content'
import WindowView from './view/window/view'
const { ipcRenderer, remote } = require('electron')

export default function main() {
  const components = [
    'core:file:reader'
  ]

  const content = new Content(
    WindowView,
    'window.main.css', {
      components
    })
  content.load().then(() => {
    ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
  })
}
