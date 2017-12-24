import { ipcRenderer, remote } from 'electron'

const fields = Symbol('fields')
const logLevels =
  new Map([['debug', 0], ['info', 1], ['warn', 2], ['error', 3]])
export default class Logger {
  constructor (config) {
    this[fields] = {
      windowId: remote.getCurrentWindow().id,
      domain: 'core',
    }
    config.watch('_.logger.level', (value) => {
      this[fields].logLevel = value
    }, 'debug')
  }

  get domain () {
    return this[fields].domain
  }

  set domain (domain) {
    this[fields].domain = domain
  }

  log (message, options = {}) {
    if (logLevels.get(this[fields].logLevel) > logLevels.get(options.level)) {
      return
    }
    const summary = (message instanceof Error)
      ? message.message
      : `${message}`
    const detail = (message instanceof Error)
      ? message.stack
      : `${message}`
    ipcRenderer.send('core:logger:message',
      remote.getCurrentWindow().id, Object.assign({
        message: {
          summary,
          detail,
        },
        timestamp: new Date(),
        domain: this[fields].domain,
      }, options))
  }

  debug (message, options = {}) {
    this.log(message, Object.assign(options, { level: 'debug' }))
  }

  info (message, options = {}) {
    this.log(message, Object.assign(options, { level: 'info' }))
  }

  warn (message, options = {}) {
    this.log(message, Object.assign(options, { level: 'warn' }))
  }

  error (message, options = {}) {
    this.log(message, Object.assign(options, { level: 'error' }))
  }
}
