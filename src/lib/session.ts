import { Disposable } from './disposable'
import { EventEmitter } from 'events'
import genet from '@genet/api'
import native from '@genet/load-module'
import objpath from 'object-path'
import path from 'path'
import titleCase from 'title-case'

export default class Session extends EventEmitter {
  private _config: any
  private _tokens: Map<string, any>
  private _libs: Set<string>
  private _fileReaders: Set<any>
  private _layerRenderers: Map<string, any>
  private _attrRenderers: Map<string, any>

  constructor(config) {
    super()
    this._config = config
    this._tokens = new Map()
    this._libs = new Set()
    this._fileReaders = new Set()
    this._layerRenderers = new Map()
    this._attrRenderers = new Map()
  }

  get tokens() {
    return this._tokens
  }

  get fileReaders() {
    return this._fileReaders
  }

  registerTokens(tokens) {
    for (const [id, data] of Object.entries(tokens)) {
      this._tokens.set(id, { ...data })
    }
    return new Disposable(() => {
      for (const id of Object.keys(tokens)) {
        this._tokens.delete(id)
      }
    })
  }

  registerFileReader(reader) {
    this._fileReaders.add(reader)
    return new Disposable(() => {
      this._fileReaders.delete(reader)
    })
  }

  registerLayerRenderer(id: string, renderer) {
    this._layerRenderers.set(id, renderer)
    return new Disposable(() => {
      this._layerRenderers.delete(id)
    })
  }

  registerAttrRenderer(id: string, renderer) {
    this._attrRenderers.set(id, renderer)
    return new Disposable(() => {
      this._attrRenderers.delete(id)
    })
  }

  registerLibrary(file: string) {
    const filePath = path.normalize(file)
    this._libs.add(filePath)
    return new Disposable(() => {
      this._libs.delete(filePath)
    })
  }

  tokenName(id: string) {
    const data = this._tokens.get(id)
    return objpath.get(data, 'name', titleCase(id.split('.').slice(-1)[0]))
  }

  layerRenderer(id: string) {
    const data = this._layerRenderers.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  attrRenderer(id: string) {
    const data = this._attrRenderers.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  async create() {
    const profile = new native.Session.Profile()
    profile.concurrency = genet.config.get('_.decoder.concurrency')
    for (const [key, value] of Object.entries(this._config.toJSON())) {
      profile.setConfig(key, JSON.stringify(value))
    }
    for (const file of this._libs) {
      try {
        profile.loadLibrary(file)
      } catch (err) {
        this.emit('error', new Error(`Filed to load ${file}: ${err.message}`))
      }
    }
    return new native.Session(profile, {})
  }
}
