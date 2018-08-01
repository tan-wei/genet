import { ipcRenderer, remote } from 'electron'
import Content from './lib/content'
import WindowView from './view/window/view'

export default function main () {
  const components = [
    'core:file:reader'
  ]

  const content = new Content(
    WindowView,
    'window.main.css',
    [
      `--components=${components.join(',')}`
    ]
  )
  content.load().then(() => {
    ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
  })
}
