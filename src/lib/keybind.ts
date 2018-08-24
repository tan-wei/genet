import { Disposable } from './disposable'
import { EventEmitter } from 'events'
import Logger from './logger'
import Mousetrap from 'mousetrap'
import deepEqual from 'deep-equal'
import Env from './env'
import fs from 'fs-extra'
import genet from '@genet/api'
import path from 'path'
import yaml from 'js-yaml'

function transformBindSet(map, binds) {
  for (const [selector, bind] of Object.entries(binds)) {
    for (const [key, action] of Object.entries(bind)) {
      map[key] = map[key] || []
      map[key].push({
        selector,
        action,
      })
    }
  }
}

export default class KeyBind extends EventEmitter {
  private _map: object
  private _bindSets: Set<any>
  private _userBindSet: any
  private _filePath: string
  private _logger: Logger
  constructor(profile: string, logger: Logger) {
    super()
    const filePath =
      path.join(Env.userProfilePath, profile, 'keybind.yml')
    fs.ensureFileSync(filePath)

    this._logger = logger
    this._filePath = filePath
    this._map = {}
    this._bindSets = new Set()

    this.load()
    fs.watchFile(filePath, () => this.load())
  }

  private load() {
    let bind = null
    try {
      bind = yaml.safeLoad(fs.readFileSync(this._filePath, 'utf8'))
    } catch (err) {
      this._logger.warn(err)
    }
    this._userBindSet = bind || {}
    this.update()
  }

  private update() {
    const map = {}
    for (const binds of this._bindSets) {
      transformBindSet(map, binds)
    }
    transformBindSet(map, this._userBindSet)
    const keys = new Set()
    for (const key of Object.keys(map)) {
      keys.add(key)
    }
    for (const key of Object.keys(this._map)) {
      keys.add(key)
    }
    for (const key of keys) {
      if (!deepEqual(map[key], this._map[key])) {
        Mousetrap.unbind(key)
        if (key in map) {
          const combo = process.platform === 'darwin'
            ? key
            : key.replace(/\bcommand\b/, 'ctrl')
          Mousetrap.bind(combo, (event) => {
            for (const binds of this._map[key]) {
              if (event.target.matches(binds.selector)) {
                genet.action.global.emit(binds.action)
                event.preventDefault()
                event.stopPropagation()
                break
              }
            }
          })
        }
      }
    }
    this._map = map
    this.emit('update')
  }

  register(binds) {
    this._bindSets.add(binds)
    this.update()
    return new Disposable(() => {
      this._bindSets.delete(binds)
      this.update()
    })
  }

  get keymap() {
    return this._map
  }
}
