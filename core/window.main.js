import { ipcRenderer, remote } from 'electron'
import Content from './content'
import RootView from './root-view'

const content = new Content(RootView)
content.load().then(() => {
  ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
})
