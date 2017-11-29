import { ipcRenderer, remote } from 'electron'
import Content from './lib/content'
import LogView from './view/log/log-view'

ipcRenderer.send('core:logger:register',
  remote.getCurrentWindow().id, remote.getCurrentWebContents().id)

const components = [
  'core:style'
]

const content = new Content(
  LogView,
  'log.less',
  [
    `--components=${components.join(',')}`,
    '--loggerDomain=log',
    '--contextMenu'
  ])
content.load()
