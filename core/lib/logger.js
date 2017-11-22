import { ipcRenderer, remote } from 'electron'
import util from 'util'

const fields = Symbol('fields')
export default class Logger {
  constructor () {
    this[fields] = { windowId: remote.getCurrentWindow().id }
  }

  log (level, format, ...args) {
    ipcRenderer.send('core:logger:message',
      remote.getCurrentWindow().id, {
        level,
        message: util.format(format, ...args),
        timestamp: new Date(),
      })
  }

  debug (format, ...args) {
    this.log('debug', format, ...args)
  }

  info (format, ...args) {
    this.log('info', format, ...args)
  }

  warn (format, ...args) {
    this.log('warn', format, ...args)
  }

  error (format, ...args) {
    this.log('error', format, ...args)
  }
}
