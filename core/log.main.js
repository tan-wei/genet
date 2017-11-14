import { ipcRenderer, remote } from 'electron'
import Content from './lib/content'
import LogView from './view/log/log-view'

ipcRenderer.send('core:logger:register',
  remote.getCurrentWindow().id, remote.getCurrentWebContents().id)

const content = new Content(LogView, 'preference.less')
content.load()
