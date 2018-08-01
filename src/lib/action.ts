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

  removeAllListeners (channel: string) {
    ipcRenderer.removeAllListeners(`${channel} #${windowId}`)
  }

  emit (channel, ...args) {
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
