import { Disposable } from 'disposables'
import deepEqual from 'deep-equal'
import env from './env'
import fs from 'fs'
import mkpath from 'mkpath'
import objpath from 'object-path'
import path from 'path'
import schemaDefault from './schema-default'
import touch from 'touch'
import { validate } from 'jsonschema'
import yaml from 'js-yaml'

const fields = Symbol('fields')
export default class Config {
  constructor (profile, name) {
    const filePath =
      path.join(env.userProfilePath, profile, `${name}.yml`)
    mkpath.sync(path.dirname(filePath))
    touch.sync(filePath)

    const schema = schemaDefault[name] || {}
    this[fields] = {
      tree: {},
      schema,
      schemaSet: new Set(),
      listeners: [],
      filePath,
      write: () =>
        fs.writeFileSync(
          this[fields].filePath, yaml.safeDump(this[fields].tree)),
      update: (oldTree) => {
        const { listeners } = this[fields]
        for (const listener of listeners) {
          const value = objpath.get(this[fields].tree, listener.id)
          const oldValue = objpath.get(oldTree, listener.id)
          if (!deepEqual(value, oldValue)) {
            listener.callback(value, oldValue)
          }
        }
      },
      load: (update = false) => {
        let tree = null
        try {
          tree = yaml.safeLoad(fs.readFileSync(filePath, 'utf8'))
        } catch (err) {
          console.warn(err)
        }
        const oldTree = this[fields].tree || {}
        this[fields].tree = tree || {}
        if (update) {
          this[fields].update(oldTree)
        }
      },
    }

    this[fields].load(false)
    fs.watchFile(filePath, () => this[fields].load(true))
  }

  registerSchema (schema) {
    const { schemaSet } = this[fields]
    schemaSet.add(schema)
    return new Disposable(() => {
      schemaSet.delete(schema)
    })
  }

  get schema () {
    const { schema, schemaSet } = this[fields]
    return Object.assign({}, schema, ...schemaSet)
  }

  get (id, defaultValue) {
    const { tree } = this[fields]
    let value = objpath.get(tree, id)
    if (typeof value !== 'undefined') {
      if (!(id in this.schema) ||
        validate(value, this.schema[id]).errors.length === 0) {
        return value
      }
    }
    if (id in this.schema) {
      value = this.schema[id].default
    }
    if (typeof value !== 'undefined') {
      return value
    }
    return defaultValue
  }

  set (id, value) {
    const { tree, update, write } = this[fields]
    let defaultValue = null
    if (id in this.schema) {
      const result = validate(value, this.schema[id])
      if (result.errors.length > 0) {
        throw result.errors[0]
      }
      defaultValue = this.schema[id].default
    }
    if (!deepEqual(value, objpath.get(tree, id))) {
      const oldTree = Object.assign({}, tree)
      if (deepEqual(value, objpath.get(defaultValue, id))) {
        objpath.del(tree, id)
      } else {
        objpath.set(tree, id, value)
      }
      update(oldTree)
      write()
    }
  }

  del (id) {
    const { tree } = this[fields]
    objpath.del(tree, id)
  }

  watch (id, callback, defaultValue) {
    const { listeners } = this[fields]
    listeners.push({
      id,
      callback,
    })
    if (typeof defaultValue !== 'undefined') {
      const value = this.get(id, defaultValue)
      // eslint-disable-next-line callback-return
      callback(value, defaultValue)
    }
    return new Disposable(() => {
      this[fields].listeners =
        listeners.filter((handler) => handler.callback !== callback)
    })
  }
}
