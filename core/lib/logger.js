/* eslint no-console: "off" */

import { remote } from 'electron'
import throttle from 'lodash.throttle'

const dumpLogs = throttle((data) => {
  if (data.logs.length > 0) {
    for (const log of data.logs) {
      switch (log.level) {
        case 'info':
          console.info(log.message)
          break
        case 'debug':
          console.debug(log.message)
          break
        case 'warn':
          console.warn(log.message)
          break
        case 'error':
          console.error(log.message)
          break
        default:
          console.log(log.message)
          break
      }
    }
    data.logs = []
  }
}, 100)

const fields = Symbol('fields')
export default class Logger {
  constructor (config) {
    this[fields] = {
      windowId: remote.getCurrentWindow().id,
      domain: 'core',
      logs: [],
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
    this[fields].logs.push({
      message,
      level: options.level,
    })
    dumpLogs(this[fields])
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
