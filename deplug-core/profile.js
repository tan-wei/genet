import { ipcRenderer, remote } from 'electron'
import { EventEmitter } from 'events'
import config from './config'
import fs from 'fs'
import glob from 'glob'
import log from 'electron-log'
import merge from 'lodash.merge'
import mkpath from 'mkpath'
import objpath from 'object-path'
import options from './options'
import path from 'path'
import yaml from 'js-yaml'

const profileRegistory = {}
const defaults = {}
let currentProfile = 'default'

const { webContents } = remote
export default class Profile extends EventEmitter {
  constructor (profile) {
    super()
    this.profileDir = path.join(config.userProfilePath, profile)
    this.configFile = path.join(this.profileDir, 'config.yml')
    this.configObject = {}

    try {
      this.configObject = yaml.safeLoad(fs.readFileSync(this.configFile))
    } catch (err) {
      log.warn(err)
    }

    ipcRenderer.on('updated', (event, opath, value) => {
      if (typeof value === 'undefined') {
        objpath.del(this.configObject, opath)
      } else {
        objpath.set(this.configObject, opath, value)
      }
      this.emit('updated', opath, value)
    })
  }

  static get list () {
    return glob.sync(path.join(config.userProfilePath, '*/config.yml'))
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

  get (opath, defaultValue) {
    const value = objpath.get(this.configObject, opath)
    if (typeof value === 'undefined') {
      if (typeof defaultValue !== 'undefined') {
        return defaultValue
      } else if (objpath.has(defaults, opath)) {
        return objpath.get(defaults, opath)
      }
    } else {
      return value
    }
    return null
  }

  delete (opath) {
    objpath.del(this.configObject, opath)
    for (const wc of webContents.getAllWebContents()) {
      wc.send('updated', opath)
    }
    this.writeSync()
  }

  set (opath, value) {
    if (typeof value === 'undefined') {
      objpath.del(this.configObject, opath)
    } else {
      objpath.set(this.configObject, opath, value)
    }
    for (const wc of webContents.getAllWebContents()) {
      wc.send('updated', opath, value)
    }
    this.writeSync()
  }

  static setDefault (opath, value) {
    objpath.set(defaults, opath, value)
  }

  static get globalOptions () {
    return options
  }

  writeSync () {
    mkpath.sync(path.dirname(this.configFile))
    fs.writeFileSync(this.configFile, yaml.safeDump(this.configObject))
  }
}

for (const [id, opt] of Object.entries(options)) {
  if ('default' in opt) {
    Profile.setDefault(`_.${id}`, opt.default)
  }
}
