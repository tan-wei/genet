import { Disposable } from './disposable'
import deepEqual from 'deep-equal'
import Env from './env'
import fs from 'fs-extra'
import objpath from 'object-path'
import path from 'path'
import schemaDefault from './schema-default'
import { validate } from 'jsonschema'
import yaml from 'js-yaml'

interface Listener {
  id: string
  callback: (value: any, defaultValue: any) => void
}

export default class Config {
  private _tree: object
  private _schema: object
  private _schemaSet: Set<any>
  private _listeners: Listener[]
  private _filePath: string

  private _write: () => void
  private _update: (object) => void
  private _load: (boolean) => void

  constructor(profile: string, name: string) {
    const filePath =
      path.join(Env.userProfilePath, profile, `${name}.yml`)
    fs.ensureFileSync(filePath)

    this._tree = {}
    this._schema = schemaDefault[name] || {}
    this._schemaSet = new Set()
    this._listeners = []
    this._filePath = filePath
    this._write = () =>
      fs.writeFileSync(
        this._filePath, yaml.safeDump(this._tree))
    this._update = (oldTree) => {
      for (const listener of this._listeners) {
        const value = objpath.get(this._tree, listener.id)
        const oldValue = objpath.get(oldTree, listener.id)
        if (!deepEqual(value, oldValue)) {
          listener.callback(value, oldValue)
        }
      }
    }
    this._load = (update = false) => {
      let tree = null
      try {
        tree = yaml.safeLoad(fs.readFileSync(filePath, 'utf8'))
      } catch (err) {
        // eslint-disable-next-line no-console
        console.warn(err)
      }
      const oldTree = this._tree || {}
      this._tree = tree || {}
      if (update) {
        this._update(oldTree)
      }
    }

    this._load(false)
  }

  registerSchema(schema) {
    this._schemaSet.add(schema)
    return new Disposable(() => {
      this._schemaSet.delete(schema)
    })
  }

  get schema(): object {
    return Object.assign({}, this._schema, ...this._schemaSet)
  }

  get(id: string, defaultValue?: any) {
    let value = objpath.get(this._tree, id)
    if (typeof value !== 'undefined') {
      if (!(id in this.schema) ||
        validate(value, this.schema[id]).errors.length === 0) {
        return Object.freeze(value)
      }
    }
    if (id in this.schema) {
      value = this.schema[id].default
    }
    if (typeof value !== 'undefined') {
      return Object.freeze(value)
    }
    return defaultValue
  }

  set(id: string, value: any) {
    let defaultValue = null
    if (id in this.schema) {
      const result = validate(value, this.schema[id])
      if (result.errors.length > 0) {
        throw result.errors[0]
      }
      defaultValue = this.schema[id].default
    }
    if (!deepEqual(value, objpath.get(this._tree, id))) {
      const oldTree = Object.assign({}, this._tree)
      if (deepEqual(value, objpath.get(defaultValue, id))) {
        objpath.del(this._tree, id)
      } else {
        objpath.set(this._tree, id, value)
      }
      this._update(oldTree)
      this._write()
    }
  }

  del(id: string) {
    objpath.del(this._tree, id)
  }

  watch(id: string, callback: (value: any, defaultValue: any) => void, defaultValue?: any) {
    if (this._listeners.length === 0) {
      fs.watchFile(this._filePath, () => this._load(true))
    }
    this._listeners.push({
      id,
      callback,
    })
    if (typeof defaultValue !== 'undefined') {
      const value = this.get(id, defaultValue)
      // eslint-disable-next-line callback-return
      callback(value, defaultValue)
    }
    return new Disposable(() => {
      this._listeners =
        this._listeners.filter((handler) => handler.callback !== callback)
      if (this._listeners.length === 0) {
        fs.unwatchFile(this._filePath, () => this._load(true))
      }
    })
  }

  toJSON() {
    const obj = {}
    for (const [key, value] of Object.entries(this.schema)) {
      if ('default' in value) {
        obj[key] = value.default
      }
      const val = objpath.get(this._tree, key)
      if (typeof val !== 'undefined') {
        obj[key] = val
      }
    }
    return obj
  }
}
