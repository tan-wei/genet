import { ipcRenderer, remote } from 'electron'
import Content from './lib/content'
import WindowView from './view/window-view'

const content = new Content(WindowView)
content.load().then(() => {
  ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
})
