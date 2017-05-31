import { ipcRenderer, remote } from 'electron'
import { EventEmitter } from 'events'
import config from './config'
import denodeify from 'denodeify'
import fs from 'fs'
import glob from 'glob'
import jsonfile from 'jsonfile'
import log from 'electron-log'
import merge from 'lodash.merge'
import objpath from 'object-path'
import options from './options'
import path from 'path'

const profileRegistory = {}
const globalDefaults = {}
const pluginDefaults = {}
const currentProfile = 'default'

const { webContents } = remote
function createHandler (plugin = null) {
  const proto =
    Object.keys(EventEmitter.prototype).concat(Object.keys(Object.prototype))
  return {
    get: (target, name) => {
      if (name === '$$dir') {
        return target.profileDir
      } else if (name === '$$object') {
        return merge(Object.assign({ '_': globalDefaults }, pluginDefaults),
          target.pluginObject)
      } else if (name.startsWith('$')) {
        return new Proxy(target, createHandler(name.substr(1)))
      } else if (proto.includes(name)) {
        return target[name]
      } else if (plugin === null) {
        return target.getGlobal(name)
      }
      return target.getPlugin(plugin, name)
    },
    has: (target, name) => {
      if (proto.includes(name)) {
        return true
      } else if (plugin === null) {
        return target.hasGlobal(name)
      }
      return target.hasPlugin(plugin, name)
    },
    deleteProperty: (target, name) => {
      if (proto.includes(name)) {
        Reflect.deleteProperty(target, name)
      } else if (plugin === null) {
        target.deleteGlobal(name)
      } else {
        target.deletePlugin(plugin, name)
      }
      return true
    },
    set: (target, name, value) => {
      if (proto.includes(name)) {
        target[name] = value
      } else if (plugin === null) {
        target.setGlobal(name, value)
      } else {
        target.setPlugin(plugin, name, value)
      }
      return true
    },
  }
}

export default class Profile extends EventEmitter {
  constructor (profile) {
    super()
    this.profileDir = path.join(config.userProfilePath, profile)
    this.configFile = path.join(this.profileDir, 'config.json')
    this.globalObject = {}
    this.pluginObject = {}

    try {
      this.pluginObject = jsonfile.readFileSync(this.configFile)
      this.globalObject = this.pluginObject._ || {}
    } catch (err) {
      log.warn(err)
    }

    ipcRenderer.on('global-updated', (event, opath, value) => {
      if (typeof value === 'undefined') {
        objpath.del(this.globalObject, opath)
      } else {
        objpath.set(this.globalObject, opath, value)
      }
      this.emit('global-updated', opath, value)
    })
    ipcRenderer.on('plugin-updated', (event, id, opath, value) => {
      if (!(id in this.pluginObject)) {
        this.pluginObject[id] = {}
      }
      if (typeof value === 'undefined') {
        objpath.del(this.pluginObject[id], opath)
      } else {
        objpath.set(this.pluginObject[id], opath, value)
      }
      this.emit('plugin-updated', id, opath, value)
    })
  }

  static get list () {
    return glob.sync(path.join(config.userProfilePath, '*/config.json'))
      .map(path.dirname)
      .map((dir) => path.basename(dir))
  }

  static get default () {
    return this.fromId('default')
  }

  static get current () {
    return this.fromId(currentProfile)
  }

  static fromId (id) {
    if (!(id in profileRegistory)) {
      profileRegistory[id] = new Profile(id)
    }
    return new Proxy(profileRegistory[id], createHandler())
  }

  getGlobal (opath) {
    const value = objpath.get(this.globalObject, opath)
    if (typeof value === 'undefined') {
      return objpath.get(globalDefaults, opath, null)
    }
    return value
  }

  hasGlobal (opath) {
    return objpath.has(this.globalObject, opath)
  }

  deleteGlobal (opath) {
    objpath.del(this.globalObject, opath)
    for (const wc of webContents.getAllWebContents()) {
      wc.send('global-updated', opath)
    }
    this.writeSync()
  }

  setGlobal (opath, value) {
    if (typeof value === 'undefined') {
      objpath.del(this.globalObject, opath)
    } else {
      objpath.set(this.globalObject, opath, value)
    }
    for (const wc of webContents.getAllWebContents()) {
      wc.send('global-updated', opath, value)
    }
    this.writeSync()
  }

  getPlugin (id, opath) {
    const value = objpath.get(this.pluginObject[id] || {}, opath)
    if (typeof value === 'undefined') {
      if (id in pluginDefaults) {
        return objpath.get(pluginDefaults[id], opath, null)
      }
    } else {
      return value
    }
    return null
  }

  hasPlugin (id, opath) {
    if (id in this.pluginObject) {
      return objpath.has(this.pluginObject[id], opath)
    }
    return false
  }

  deletePlugin (id, opath) {
    if (id in this.pluginObject) {
      objpath.del(this.pluginObject[id], opath)
      for (const wc of webContents.getAllWebContents()) {
        wc.send('plugin-updated', id, opath)
      }
    }
    this.writeSync()
  }

  setPlugin (id, opath, value) {
    if (!(id in this.pluginObject)) {
      this.pluginObject[id] = {}
    }
    if (typeof value === 'undefined') {
      objpath.del(this.pluginObject[id], opath)
    } else {
      objpath.set(this.pluginObject[id], opath, value)
    }
    for (const wc of webContents.getAllWebContents()) {
      wc.send('plugin-updated', id, opath, value)
    }
    this.writeSync()
  }

  static setPluginDefault (id, opath, value) {
    if (!(id in pluginDefaults)) {
      pluginDefaults[id] = {}
    }
    objpath.set(pluginDefaults[id], opath, value)
  }

  static setGlobalDefault (opath, value) {
    objpath.set(globalDefaults, opath, value)
  }

  static get globalOptions () {
    return options
  }

  async write () {
    const write = denodeify(fs.writeFile)
    const object = Object.assign(this.pluginObject, { '_': this.globalObject })
    return write(this.configFile,
      JSON.stringify(
        object, null, ' '))
  }

  writeSync () {
    const object = Object.assign(this.pluginObject, { '_': this.globalObject })
    fs.writeFileSync(this.configFile,
      JSON.stringify(object, null, ' '))
  }
}

for (const opt of options) {
  if ('default' in opt) {
    Profile.setGlobalDefault(opt.id, opt.default)
  }
}
