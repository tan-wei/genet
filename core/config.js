import configDefault from './config-default'
import deepEqual from 'deep-equal'
import env from './env'
import fs from 'fs'
import mkpath from 'mkpath'
import objpath from 'object-path'
import path from 'path'
import touch from 'touch'
import yaml from 'js-yaml'

const fields = Symbol('fields')
export default class Config {
  constructor (profile, name) {
    const filePath =
      path.join(env.userProfilePath, profile, `${name}.yml`)
    mkpath.sync(path.dirname(filePath))
    touch.sync(filePath)

    this[fields] = {
      tree: {},
      defaultTree: configDefault[name] || {},
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

  get (id, defaultValue) {
    const { defaultTree, tree } = this[fields]
    let value = objpath.get(tree, id)
    if (typeof value !== 'undefined') {
      return value
    }
    value = objpath.get(defaultTree, id)
    if (typeof value !== 'undefined') {
      return value
    }
    return defaultValue
  }

  set (id, value) {
    const { defaultTree, tree, update, write } = this[fields]
    if (!deepEqual(value, objpath.get(tree, id))) {
      const oldTree = Object.assign({}, tree)
      if (deepEqual(value, objpath.get(defaultTree, id))) {
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

  watch (id, callback) {
    const { listeners } = this[fields]
    listeners.push({
      id,
      callback,
    })
  }
}
