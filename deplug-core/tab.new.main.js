import { remote } from 'electron'
const { webContents } = remote
import deplug from './deplug.new'

const packageWhiteList = [
  'mithril',
  'moment',
  'path',
  'mithril',
  'deplug',
  'plugkit',
  'lodash.throttle'
]
export default function (argv, tab) {

  const nodeRequire = require
  const nodeProcess = process

  console.log(deplug(argv))

  const contents = remote.getCurrentWebContents()
  contents.on('dom-ready', () => {
    global.require = (name) => {
      if (!packageWhiteList.includes(name)) {
        console.warn(name)
      }
      return nodeRequire(name)
    }
    global.process = {
      execPath: nodeProcess.execPath,
      platform: nodeProcess.platform,
    }
    for (const wc of webContents.getAllWebContents()) {
      wc.send('tab-deplug-loaded', 50)
    }
  })
}
