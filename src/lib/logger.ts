/* eslint no-console: "off" */

import { remote } from 'electron'
import throttle from 'lodash.throttle'

enum Level {
  Debug = "debug",
  Info = "info",
  Warn = "warn",
  Error = "error",
}

interface Options {
  level?: Level;
}

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

  log (message, options: Options = {}) {
    this[fields].logs.push({
      message,
      level: options.level,
    })
    dumpLogs(this[fields])
  }

  debug (message, options: Options = {}) {
    this.log(message, Object.assign(options, { level: Level.Debug }))
  }

  info (message, options: Options = {}) {
    this.log(message, Object.assign(options, { level: Level.Info }))
  }

  warn (message, options: Options = {}) {
    this.log(message, Object.assign(options, { level: Level.Warn }))
  }

  error (message, options: Options = {}) {
    this.log(message, Object.assign(options, { level: Level.Error }))
  }
}
