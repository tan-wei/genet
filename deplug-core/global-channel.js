import { ipcRenderer, remote } from 'electron'
const { webContents } = remote
const channelPrefix = `deplug-channel@${remote.getCurrentWindow().id}@`
export default class GlobalChannel {
  static on (channel, listener) {
    ipcRenderer.on(channelPrefix + channel, listener)
  }

  static once (channel, listener) {
    ipcRenderer.once(channelPrefix + channel, listener)
  }

  static removeListener (channel, listener) {
    ipcRenderer.removeListener(channelPrefix + channel, listener)
  }

  static removeAllListeners (channel) {
    if (typeof channel === 'string') {
      ipcRenderer.removeAllListeners(channelPrefix + channel)
    } else {
      ipcRenderer.removeAllListeners()
    }
  }

  static emit (channel, ...args) {
    for (const wc of webContents.getAllWebContents()) {
      wc.send(channelPrefix + channel, ...args)
    }
  }
}
