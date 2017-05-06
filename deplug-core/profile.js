import { ipcRenderer, remote } from 'electron'
import { EventEmitter } from 'events'
import config from './config'
import denodeify from 'denodeify'
import fs from 'fs'
import glob from 'glob'
import jsonfile from 'jsonfile'
import log from 'electron-log'
import mkpath from 'mkpath'
import objpath from 'object-path'
import path from 'path'

const { webContents } = remote
function createHandler (plugin = null) {
  const proto =
    Object.keys(EventEmitter.prototype).concat(Object.keys(Object.prototype))
  return {
    get: (target, name) => {
      if (name === '$$dir') {
        return target.profileDir
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

const profileRegistory = {}
const pluginDefaults = {}
const currentProfile = 'default'
export default class Profile extends EventEmitter {
  constructor (profile) {
    super()
    this.profileDir = path.join(config.userProfilePath, profile)
    this.pluginsDir = path.join(this.profileDir, 'plugins')
    mkpath.sync(this.pluginsDir)
    this.globalFile = path.join(this.profileDir, 'global.json')

    try {
      this.globalObject = jsonfile.readFileSync(this.globalFile)
    } catch (err) {
      log.warn(err)
      this.globalObject = {}
    }

    this.pluginObject = {}
    const plugins = glob.sync(path.join(this.pluginsDir, '*.json'))
    for (const json of plugins) {
      try {
        this.pluginObject[path.basename(json, '.json')] =
          jsonfile.readFileSync(json)
      } catch (err) {
        log.warn(err)
      }
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
    return objpath.get(this.globalObject, opath, null)
  }

  hasGlobal (opath) {
    return objpath.has(this.globalObject, opath)
  }

  deleteGlobal (opath) {
    objpath.del(this.globalObject, opath)
    for (const wc of webContents.getAllWebContents()) {
      wc.send('global-updated', opath)
    }
    this.write()
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
    this.write()
  }

  getPlugin (id, opath) {
    if (id in this.pluginObject) {
      const value = objpath.get(this.pluginObject[id], opath)
      if (typeof value === 'undefined') {
        if (id in pluginDefaults) {
          return objpath.get(pluginDefaults[id], opath, null)
        }
      } else {
        return value
      }
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
    this.write()
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
    this.write()
  }

  static setPluginDefault (id, opath, value) {
    if (!(id in pluginDefaults)) {
      pluginDefaults[id] = {}
    }
    objpath.set(pluginDefaults[id], opath, value)
  }

  async write () {
    const write = denodeify(fs.writeFile)
    await write(this.globalFile, JSON.stringify(this.globalObject))
    const tasks = []
    for (const id in this.pluginObject) {
      const jsonFile = path.join(this.pluginsDir, `${id}.json`)
      tasks.push(write(jsonFile, JSON.stringify(this.pluginObject[id])))
    }
    return Promise.all(tasks)
  }

  writeSync () {
    fs.writeFileSync(this.globalFile, JSON.stringify(this.globalObject))
    for (const id in this.pluginObject) {
      const jsonFile = path.join(this.pluginsDir, `${id}.json`)
      fs.writeFileSync(jsonFile, JSON.stringify(this.pluginObject[id]))
    }
  }
}
