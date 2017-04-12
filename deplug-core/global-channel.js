import { ipcRenderer, remote } from 'electron'
const { webContents, } = remote

let count = 0
export default class GlobalChannel {
  constructor () {
    count += 1
    this.channelPrefix = `deplug-channel-${count}@`
  }

  on (channel, listener) {
    ipcRenderer.on(this.channelPrefix + channel, listener)
  }

  once (channel, listener) {
    ipcRenderer.once(this.channelPrefix + channel, listener)
  }

  removeListener (channel, listener) {
    ipcRenderer.removeListener(this.channelPrefix + channel, listener)
  }

  removeAllListeners (channel) {
    if (typeof channel === 'string') {
      ipcRenderer.removeAllListeners(this.channelPrefix + channel)
    } else {
      ipcRenderer.removeAllListeners()
    }
  }

  emit (channel, ...args) {
    for (const wc of webContents.getAllWebContents()) {
      wc.send(this.channelPrefix + channel, ...args)
    }
  }
}
