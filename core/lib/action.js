import { ipcRenderer, remote } from 'electron'
import { EventEmitter } from 'events'

const windowId = remote.getCurrentWindow().id
const { webContents } = remote
class GlobalAction {
  on (channel, listener) {
    ipcRenderer.on(`${channel} #${windowId}`, listener)
  }

  once (channel, listener) {
    ipcRenderer.once(`${channel} #${windowId}`, listener)
  }

  removeListener (channel, listener) {
    ipcRenderer.removeListener(`${channel} #${windowId}`, listener)
  }

  removeAllListeners (channel) {
    if (typeof channel === 'string') {
      ipcRenderer.removeAllListeners(`${channel} #${windowId}`)
    } else {
      ipcRenderer.removeAllListeners()
    }
  }

  emit (channel, ...args) {
    for (const wc of webContents.getAllWebContents()) {
      wc.send(`${channel} #${windowId}`, ...args)
    }
  }
}

export default class Action extends EventEmitter {
  constructor () {
    super()
    this.global = new GlobalAction()
  }
}
