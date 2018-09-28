/* eslint no-console: "off" */

const { remote } = require('electron')
import throttle from 'lodash.throttle'
import Config from './config'

enum Level {
  Debug = "debug",
  Info = "info",
  Warn = "warn",
  Error = "error",
}

interface Options {
  level?: Level;
}

interface Log {
  message: string
  level?: Level;
}

const dumpLogs = throttle((logs: Log[]) => {
  if (logs.length > 0) {
    for (const log of logs) {
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
    logs.splice(0)
  }
}, 100)

export default class Logger {
  private _domain: string
  private _logs: Log[]
  constructor() {
    this._domain = 'core'
    this._logs = []
  }

  get domain() {
    return this._domain
  }

  set domain(domain: string) {
    this._domain = domain
  }

  private log(message: string, options: Options = {}) {
    this._logs.push({
      message,
      level: options.level,
    })
    dumpLogs(this._logs)
  }

  debug(message: string, options: Options = {}) {
    this.log(message, { ...options, level: Level.Debug })
  }

  info(message: string, options: Options = {}) {
    this.log(message, { ...options, level: Level.Info })
  }

  warn(message: string, options: Options = {}) {
    this.log(message, { ...options, level: Level.Warn })
  }

  error(message: string, options: Options = {}) {
    this.log(message, { ...options, level: Level.Error })
  }
}
