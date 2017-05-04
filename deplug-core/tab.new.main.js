import { remote } from 'electron'
const { webContents } = remote

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
  const contents = remote.getCurrentWebContents()
  const nodeRequire = require
  const nodeProcess = process
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
