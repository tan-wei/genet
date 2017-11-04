import { ipcRenderer, remote } from 'electron'
import Content from './lib/content'
import RootView from './view/root-view'

const content = new Content(RootView)
content.load().then(() => {
  ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
})
