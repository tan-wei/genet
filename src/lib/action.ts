import { EventEmitter } from 'events'
const { ipcRenderer, remote } = require('electron')

const windowId = remote.getCurrentWindow().id
const { webContents } = remote
class GlobalAction {
  on (channel: string, listener: (...args) => void) {
    ipcRenderer.on(`${channel} #${windowId}`, listener)
  }

  once (channel: string, listener: (...args) => void) {
    ipcRenderer.once(`${channel} #${windowId}`, listener)
  }

  removeListener (channel: string, listener: (...args) => void) {
    ipcRenderer.removeListener(`${channel} #${windowId}`, listener)
  }

  removeAllListeners (channel: string) {
    ipcRenderer.removeAllListeners(`${channel} #${windowId}`)
  }

  emit (channel: string, ...args) {
    for (const wc of webContents.getAllWebContents()) {
      wc.send(`${channel} #${windowId}`, ...args)
    }
  }
}

export default class Action extends EventEmitter {
  public global: GlobalAction

  constructor () {
    super()
    this.global = new GlobalAction()
  }
}
