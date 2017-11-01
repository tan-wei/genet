import Mousetrap from 'mousetrap'
import deepEqual from 'deep-equal'
import env from './env'
import fs from 'fs'
import mkpath from 'mkpath'
import path from 'path'
import touch from 'touch'
import yaml from 'js-yaml'

const fields = Symbol('fields')
export default class KeyBind {
  constructor (profile) {
    const filePath =
      path.join(env.userProfilePath, profile, 'keybind.yml')
    mkpath.sync(path.dirname(filePath))
    touch.sync(filePath)

    this[fields] = {
      filePath,
      map: {},
      load: () => {
        let bind = null
        try {
          bind = yaml.safeLoad(fs.readFileSync(filePath, 'utf8'))
        } catch (err) {
          console.warn(err)
        }
        this[fields].bind = bind || {}
        this.update()
      },
    }

    this[fields].load()
    fs.watchFile(filePath, () => this[fields].load())
  }

  update () {
    const map = {}
    for (const [selector, bind] of Object.entries(this[fields].bind)) {
      for (const [key, command] of Object.entries(bind)) {
        map[key] = map[key] || []
        map[key].push({
          selector,
          command,
        })
      }
    }
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
}
