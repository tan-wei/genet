import { Disposable } from 'disposables'
import native from '@genet/load-module'
import objpath from 'object-path'
import path from 'path'
import titleCase from 'title-case'

const fields = Symbol('fields')
export default class Session {
  constructor (config) {
    this[fields] = {
      config,
      tokens: new Map(),
      libs: new Set(),
      layerRenderers: new Map(),
      attrRenderers: new Map(),
      attrMacros: new Map(),
      filterMacros: new Set(),
      samples: new Set(),
    }
    setTimeout(() => {
      const { Session: SESS } = require('@genet/load-module')
      const { Profile } = SESS
      const prof = new Profile()
      prof.loadLibrary('./lib/genet-kernel/target/debug/examples/libeth.dylib')
      prof.loadLibrary('./lib/genet-kernel/target/debug/examples/libreader.dylib')
      prof.loadLibrary('./package/pcap/crates/pcap-reader/target/release/libpcap_reader.dylib')
      const sess = new SESS(prof)
      sess.setFilter('main', '_.src')
      const input = sess.createReader('pcap', JSON.stringify({
        cmd: './package/pcap/crates/pcap-cli/target/release/pcap_cli',
        args: ['capture', 'en0'],
        link: 1,
      }))
      sess.on('event', (e) => {
        console.log(e)
        input.dispose()
        console.log(sess.frames(0, 10).map((f) => f.root))
      })
      console.log(sess)
    }, 0)
  }

  get tokens () {
    return this[fields].tokens
  }

  registerTokens (tokens) {
    for (const [id, data] of Object.entries(tokens)) {
      this[fields].tokens.set(id, data)
    }
    return new Disposable(() => {
      for (const id of Object.keys(tokens)) {
        this[fields].tokens.delete(id)
      }
    })
  }

  registerLayerRenderer (id, renderer) {
    this[fields].layerRenderers.set(id, renderer)
    return new Disposable(() => {
      this[fields].layerRenderers.delete(id)
    })
  }

  registerAttrRenderer (id, renderer) {
    this[fields].attrRenderers.set(id, renderer)
    return new Disposable(() => {
      this[fields].attrRenderers.delete(id)
    })
  }

  registerAttrMacro (id, macro) {
    this[fields].attrMacros.set(id, macro)
    return new Disposable(() => {
      this[fields].attrMacros.delete(id)
    })
  }

  registerFilterMacro (macro) {
    this[fields].filterMacros.add(macro)
    return new Disposable(() => {
      this[fields].filterMacros.delete(macro)
    })
  }

  registerSample (sample) {
    this[fields].samples.add(sample)
    return new Disposable(() => {
      this[fields].samples.delete(sample)
    })
  }

  registerLibrary (file) {
    const filePath = path.normalize(file)
    this[fields].libs.add(filePath)
    return new Disposable(() => {
      this[fields].libs.delete(filePath)
    })
  }

  tokenName (id) {
    const data = this[fields].tokens.get(id)
    return objpath.get(data, 'name', titleCase(id.split('.').slice(-1)[0]))
  }

  layerRenderer (id) {
    const data = this[fields].layerRenderers.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  attrRenderer (id) {
    const data = this[fields].attrRenderers.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  attrMacro (id) {
    const data = this[fields].attrMacros.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return null
  }

  async create () {
    const { config, libs } = this[fields]
    const profile = new native.Session.Profile()
    for (const [key, value] of Object.entries(config.toJSON())) {
      profile.setConfig(key, JSON.stringify(value))
    }
    for (const file of libs) {
      profile.loadLibrary(file)
    }
    return new native.Session(profile)
  }

  createFilterCompiler () {
    const { filterMacros } = this[fields]
    const filter = new native.FilterCompiler()
    filter.macros = Array.from(filterMacros)
    return filter
  }
}
