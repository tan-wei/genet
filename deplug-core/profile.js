import { ipcRenderer, remote } from 'electron'
import { EventEmitter } from 'events'
import config from './config'
import fs from 'fs'
import glob from 'glob'
import jsonfile from 'jsonfile'
import log from 'electron-log'
import merge from 'lodash.merge'
import mkpath from 'mkpath'
import objpath from 'object-path'
import options from './options'
import path from 'path'

const profileRegistory = {}
const defaults = {}
let currentProfile = 'default'

const { webContents } = remote
export default class Profile extends EventEmitter {
  constructor (profile) {
    super()
    this.profileDir = path.join(config.userProfilePath, profile)
    this.configFile = path.join(this.profileDir, 'config.json')
    this.configObject = {}

    try {
      this.configObject = jsonfile.readFileSync(this.configFile)
    } catch (err) {
      log.warn(err)
    }

    ipcRenderer.on('updated', (event, id, opath, value) => {
      if (typeof this.configObject[id] === 'undefined') {
        this.configObject[id] = {}
      }
      if (typeof value === 'undefined') {
        objpath.del(this.configObject[id], opath)
      } else {
        objpath.set(this.configObject[id], opath, value)
      }
      this.emit('updated', id, opath, value)
    })
  }

  static get list () {
    return glob.sync(path.join(config.userProfilePath, '*/config.json'))
      .map(path.dirname)
      .map((dir) => path.basename(dir))
  }

  static get current () {
    return this.fromId(this.currentId)
  }

  static get currentId () {
    return currentProfile
  }

  static set currentId (id) {
    currentProfile = id
  }

  static fromId (id) {
    if (!(id in profileRegistory)) {
      profileRegistory[id] = new Profile(id)
    }
    return profileRegistory[id]
  }

  get dir () {
    return this.profileDir
  }

  get object () {
    return merge(defaults, this.configObject)
  }

  get (id, opath, defaultValue) {
    const value = objpath.get(this.configObject[id] || {}, opath)
    if (typeof value === 'undefined') {
      if (typeof defaultValue !== 'undefined') {
        return defaultValue
      } else if (id in defaults) {
        return objpath.get(defaults[id], opath, null)
      }
    } else {
      return value
    }
    return null
  }

  delete (id, opath) {
    if (id in this.configObject) {
      objpath.del(this.configObject[id], opath)
      for (const wc of webContents.getAllWebContents()) {
        wc.send('updated', id, opath)
      }
    }
    this.writeSync()
  }

  set (id, opath, value) {
    if (typeof this.configObject[id] === 'undefined') {
      this.configObject[id] = {}
    }
    if (typeof value === 'undefined') {
      objpath.del(this.configObject[id], opath)
    } else {
      objpath.set(this.configObject[id], opath, value)
    }
    for (const wc of webContents.getAllWebContents()) {
      wc.send('updated', id, opath, value)
    }
    this.writeSync()
  }

  static setDefault (id, opath, value) {
    if (!(id in defaults)) {
      defaults[id] = {}
    }
    objpath.set(defaults[id], opath, value)
  }

  static get globalOptions () {
    return options
  }

  writeSync () {
    mkpath.sync(path.dirname(this.configFile))
    fs.writeFileSync(this.configFile,
      JSON.stringify(this.configObject, null, ' '))
  }
}

for (const [id, opt] of Object.entries(options)) {
  if ('default' in opt) {
    Profile.setDefault('_', id, opt.default)
  }
}
