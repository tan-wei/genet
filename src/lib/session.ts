import { Disposable } from './disposable'
import genet from '@genet/api'
import native from '@genet/load-module'
import objpath from 'object-path'
import path from 'path'
import titleCase from 'title-case'

const fields = Symbol('fields')
export default class Session {
  constructor(config) {
    this[fields] = {
      config,
      tokens: new Map(),
      libs: new Set(),
      fileReaders: new Set(),
      layerRenderers: new Map(),
      attrRenderers: new Map(),
      attrMacros: new Map(),
      filterMacros: new Set(),
      samples: new Set(),
    }
  }

  get tokens() {
    return this[fields].tokens
  }

  get fileReaders() {
    return this[fields].fileReaders
  }

  registerTokens(tokens) {
    for (const [id, data] of Object.entries(tokens)) {
      this[fields].tokens.set(id, data)
    }
    return new Disposable(() => {
      for (const id of Object.keys(tokens)) {
        this[fields].tokens.delete(id)
      }
    })
  }

  registerFileReader(reader) {
    this[fields].fileReaders.add(reader)
    return new Disposable(() => {
      this[fields].fileReaders.delete(reader)
    })
  }

  registerLayerRenderer(id: string, renderer) {
    this[fields].layerRenderers.set(id, renderer)
    return new Disposable(() => {
      this[fields].layerRenderers.delete(id)
    })
  }

  registerAttrRenderer(id: string, renderer) {
    this[fields].attrRenderers.set(id, renderer)
    return new Disposable(() => {
      this[fields].attrRenderers.delete(id)
    })
  }

  registerAttrMacro(id: string, macro) {
    this[fields].attrMacros.set(id, macro)
    return new Disposable(() => {
      this[fields].attrMacros.delete(id)
    })
  }

  registerFilterMacro(macro) {
    this[fields].filterMacros.add(macro)
    return new Disposable(() => {
      this[fields].filterMacros.delete(macro)
    })
  }

  registerSample(sample) {
    this[fields].samples.add(sample)
    return new Disposable(() => {
      this[fields].samples.delete(sample)
    })
  }

  registerLibrary(file: string) {
    const filePath = path.normalize(file)
    this[fields].libs.add(filePath)
    return new Disposable(() => {
      this[fields].libs.delete(filePath)
    })
  }

  tokenName(id: string) {
    const data = this[fields].tokens.get(id)
    return objpath.get(data, 'name', titleCase(id.split('.').slice(-1)[0]))
  }

  layerRenderer(id: string) {
    const data = this[fields].layerRenderers.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  attrRenderer(id: string) {
    const data = this[fields].attrRenderers.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  attrMacro(id: string) {
    const data = this[fields].attrMacros.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  async create() {
    const { config, libs } = this[fields]
    const profile = new native.Session.Profile()
    profile.concurrency = genet.config.get('_.dissector.concurrency')
    for (const [key, value] of Object.entries(config.toJSON())) {
      profile.setConfig(key, JSON.stringify(value))
    }
    for (const file of libs) {
      profile.loadLibrary(file)
    }
    return new native.Session(profile, this[fields])
  }

  createFilterCompiler() {
    const { filterMacros } = this[fields]
    const filter = new native.FilterCompiler()
    filter.macros = Array.from(filterMacros)
    return filter
  }
}
