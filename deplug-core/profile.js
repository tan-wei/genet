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

const { webContents, } = remote
function createHandler (parcel = null) {
  const proto =
    Object.keys(EventEmitter.prototype).concat(Object.keys(Object.prototype))
  return {
    get: (target, name) => {
      if (proto.includes(name)) {
        return target[name]
      } else if (parcel === null) {
        return target.getGlobal(name)
      }
      return target.getParcel(parcel, name)
    },
    has: (target, name) => {
      if (proto.includes(name)) {
        return true
      } else if (parcel === null) {
        return target.hasGlobal(name)
      }
      return target.hasParcel(parcel, name)
    },
    deleteProperty: (target, name) => {
      if (proto.includes(name)) {
        Reflect.deleteProperty(target, name)
      } else if (parcel === null) {
        target.deleteGlobal(name)
      } else {
        target.deleteParcel(parcel, name)
      }
    },
    set: (target, name, value) => {
      if (proto.includes(name)) {
        target[name] = value
      } else if (parcel === null) {
        target.setGlobal(name, value)
      } else {
        target.setParcel(parcel, name, value)
      }
    },
    apply: (target, thisArg, argumentsList) => {
      if (parcel === null && argumentsList.length === 1) {
        return new Proxy(target, createHandler(argumentsList[0]))
      }
      throw new Error('invalid arguments')
    },
  }
}

const profileRegistory = {}
const currentProfile = 'default'
export default class Profile extends EventEmitter {
  constructor (profile) {
    super()
    const profileDir = path.join(config.userProfilePath, profile)
    this.parcelsDir = path.join(profileDir, 'parcels')
    mkpath.sync(this.parcelsDir)
    this.globalFile = path.join(profileDir, 'global.json')

    try {
      this.globalObject = jsonfile.readFileSync(this.globalFile)
    } catch (err) {
      log.warn(err)
      this.globalObject = {}
    }

    this.parcelObject = {}
    const parcels = glob.sync(path.join(this.parcelsDir, '*.json'))
    for (const json of parcels) {
      try {
        this.parcelObject[path.basename(json, '.json')] =
          jsonfile.readFileSync(json)
      } catch (err) {
        log.warn(err)
      }
    }

    ipcRenderer.on('global-updated', (event, opath, value) => {
      if (value === null) {
        objpath.del(this.globalObject, opath)
      } else {
        objpath.set(this.globalObject, opath, value)
      }
      this.emit('global-updated', opath, value)
    })
    ipcRenderer.on('parcel-updated', (event, id, opath, value) => {
      if (!(id in this.parcelObject)) {
        this.parcelObject[id] = {}
      }
      if (value === null) {
        objpath.del(this.parcelObject[id], opath)
      } else {
        objpath.set(this.parcelObject[id], opath, value)
      }
      this.emit('parcel-updated', id, opath, value)
    })
    process.on('exit', this.writeSync)
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
      wc.send('global-updated', opath, null)
    }
    this.write()
  }

  setGlobal (opath, value) {
    if (value === null) {
      objpath.del(this.globalObject, opath)
    } else {
      objpath.set(this.globalObject, opath, value)
    }
    for (const wc of webContents.getAllWebContents()) {
      wc.send('global-updated', opath, value)
    }
    this.write()
  }

  getParcel (id, opath) {
    if (id in this.parcelObject) {
      return objpath.get(this.parcelObject[id], opath, null)
    }
    return null
  }

  hasParcel (id, opath) {
    if (id in this.parcelObject) {
      return objpath.has(this.parcelObject[id], opath)
    }
    return false
  }

  deleteParcel (id, opath) {
    if (id in this.parcelObject) {
      objpath.del(this.parcelObject[id], opath)
      for (const wc of webContents.getAllWebContents()) {
        wc.send('parcel-updated', id, opath, null)
      }
    }
    this.write()
  }

  setParcel (id, opath, value) {
    if (!(id in this.parcelObject)) {
      this.parcelObject[id] = {}
    }
    if (value === null) {
      objpath.del(this.parcelObject[id], opath)
    } else {
      objpath.set(this.parcelObject[id], opath, value)
    }
    for (const wc of webContents.getAllWebContents()) {
      wc.send('parcel-updated', id, opath, value)
    }
    this.write()
  }

  async write () {
    const write = denodeify(fs.writeFile)
    await write(this.globalFile, JSON.stringify(this.globalObject))
    const tasks = []
    for (const id in this.parcelObject) {
      const jsonFile = path.join(this.parcelsDir, `${id}.json`)
      tasks.push(write(jsonFile, JSON.stringify(this.parcelObject[id])))
    }
    return Promise.all(tasks)
  }

  writeSync () {
    fs.writeFileSync(this.globalFile, JSON.stringify(this.globalObject))
    for (const id in this.parcelObject) {
      const jsonFile = path.join(this.parcelsDir, `${id}.json`)
      fs.writeFileSync(jsonFile, JSON.stringify(this.parcelObject[id]))
    }
  }
}
