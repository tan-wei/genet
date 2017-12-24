import { ipcRenderer, remote } from 'electron'
import Content from './lib/content'
import WindowView from './view/window/view'

const components = [
  'core:file:importer',
  'core:file:exporter'
]

const content = new Content(
  WindowView,
  'window.less',
  [
    `--components=${components.join(',')}`
  ]
)
content.load().then(() => {
  ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
})
