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

    let tree = null
    try {
      tree = yaml.safeLoad(fs.readFileSync(filePath, 'utf8'))
    } catch (err) {
      console.warn(err)
    }

    this[fields] = {
      tree: tree || {},
      defaultTree: configDefault[name] || {},
      filePath,
      write: () =>
        fs.writeFileSync(
          this[fields].filePath, yaml.safeDump(this[fields].tree)),
    }
  }

  get (id, defaultValue) {
    let value = objpath.get(this[fields].tree, id)
    if (typeof value !== 'undefined') {
      return value
    }
    value = objpath.get(this[fields].defaultTree, id)
    if (typeof value !== 'undefined') {
      return value
    }
    return defaultValue
  }

  set (id, value) {
    if (!deepEqual(value, objpath.get(this[fields].tree, id))) {
      if (deepEqual(value, objpath.get(this[fields].defaultTree, id))) {
        objpath.del(this[fields].tree, id)
      } else {
        objpath.set(this[fields].tree, id, value)
      }
      this[fields].write()
    }
  }

  del (id) {
    objpath.del(this[fields].tree, id)
  }
}
