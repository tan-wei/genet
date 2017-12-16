import { Disposable } from 'disposables'
import Mousetrap from 'mousetrap'
import deepEqual from 'deep-equal'
import env from './env'
import fs from 'fs'
import mkpath from 'mkpath'
import path from 'path'
import touch from 'touch'
import yaml from 'js-yaml'

const fields = Symbol('fields')
function transformBindSet (map, binds) {
  for (const [selector, bind] of Object.entries(binds)) {
    for (const [key, command] of Object.entries(bind)) {
      map[key] = map[key] || []
      map[key].push({
        selector,
        command,
      })
    }
  }
}

export default class KeyBind {
  constructor (profile) {
    const filePath =
      path.join(env.userProfilePath, profile, 'keybind.yml')
    mkpath.sync(path.dirname(filePath))
    touch.sync(filePath)

    this[fields] = {
      filePath,
      map: {},
      bindSets: new Set(),
      load: () => {
        let bind = null
        try {
          bind = yaml.safeLoad(fs.readFileSync(filePath, 'utf8'))
        } catch (err) {
          // eslint-disable-next-line no-console
          console.warn(err)
        }
        this[fields].userBindSet = bind || {}
        this.update()
      },
    }

    this[fields].load()
    fs.watchFile(filePath, () => this[fields].load())
  }

  register (binds) {
    const { bindSets } = this[fields]
    bindSets.add(binds)
    this.update()
    return new Disposable(() => {
      bindSets.delete(binds)
      this.update()
    })
  }

  update () {
    const map = {}
    for (const binds of this[fields].bindSets) {
      transformBindSet(map, binds)
    }
    transformBindSet(map, this[fields].userBindSet)
    const keys = new Set()
    for (const key of Object.keys(map)) {
      keys.add(key)
    }
    for (const key of Object.keys(this[fields].map)) {
      keys.add(key)
    }
    for (const key of keys) {
      if (!deepEqual(map[key], this[fields].map[key])) {
        Mousetrap.unbind(key)
        if (key in map) {
          Mousetrap.bind(key, (event, combo) => {
            for (const binds of this[fields].map[combo]) {
              if (event.target.matches(binds.selector)) {
                event.preventDefault()
                event.stopPropagation()
                break
              }
            }
          })
        }
      }
    }
    this[fields].map = map
  }

  get keymap () {
    return this[fields].map
  }
}
