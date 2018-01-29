import { Disposable } from 'disposables'
import { SessionFactory } from '@deplug/plugkit'
import flatten from 'flat'
import jsonfile from 'jsonfile'

const fields = Symbol('fields')
export default class Session {
  constructor (config) {
    this[fields] = {
      config,
      tokens: new Map(),
      linkLayers: new Set(),
      dissectors: new Set(),
      layerRenderers: new Map(),
      attrRenderers: new Map(),
      attrMacros: new Map(),
      filterTransforms: new Set(),
      filterMacros: new Set(),
      samples: new Set(),
      importers: new Set(),
      exporters: new Set(),
      fileImporterExtensions: new Set(),
      fileExporterExtensions: new Set(),
    }
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

  registerDissector (diss) {
    this[fields].dissectors.add(diss)
    return new Disposable(() => {
      this[fields].dissectors.delete(diss)
    })
  }

  registerLinkLayer (link) {
    this[fields].linkLayers.add(link)
    return new Disposable(() => {
      this[fields].linkLayers.delete(link)
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

  registerFilterTransform (renderer) {
    this[fields].filterTransforms.add(renderer)
    return new Disposable(() => {
      this[fields].filterTransforms.delete(renderer)
    })
  }

  registerFilterMacro (macro) {
    this[fields].filterMacros.add(macro)
    return new Disposable(() => {
      this[fields].filterMacros.delete(macro)
    })
  }

  registerImporter (importer) {
    const wrapper = { importer }
    this[fields].importers.add(wrapper)
    return new Disposable(() => {
      this[fields].importers.delete(wrapper)
    })
  }

  registerExporter (exporter) {
    const wrapper = { exporter }
    this[fields].exporters.add(wrapper)
    return new Disposable(() => {
      this[fields].exporters.delete(wrapper)
    })
  }

  registerFileImporterExtensions (extensions) {
    this[fields].fileImporterExtensions.add(extensions)
    return new Disposable(() => {
      this[fields].fileImporterExtensions.delete(extensions)
    })
  }

  registerFileExporterExtensions (extensions) {
    this[fields].fileExporterExtensions.add(extensions)
    return new Disposable(() => {
      this[fields].fileExporterExtensions.delete(extensions)
    })
  }

  registerSample (sample) {
    this[fields].samples.add(sample)
    return new Disposable(() => {
      this[fields].samples.delete(sample)
    })
  }

  token (id) {
    const data = this[fields].tokens.get(id)
    if (typeof data !== 'undefined') {
      return data
    }
    return { name: id }
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

  get fileExtensions () {
    function merge (fileExtensions) {
      const map = new Map()
      for (const obj of fileExtensions) {
        for (const [ext, name] of Object.entries(obj)) {
          map.set(name, map.get(name) || new Set())
          map.get(name).add(ext)
        }
      }
      return Array.from(map.entries()).map(([name, exts]) => ({
        name,
        extensions: Array.from(exts),
      }))
    }
    return {
      importer: merge(this[fields].fileImporterExtensions),
      exporter: merge(this[fields].fileExporterExtensions),
    }
  }

  async create (ifs = '') {
    const {
      config, tokens, linkLayers,
      dissectors, filterTransforms, filterMacros,
      importers, exporters,
    } = this[fields]
    const factory = new SessionFactory()
    factory.macroPrefix = config.get('_.filter.macroPrefix', '@')
    factory.enableDebugSession = config.get('_.debug.enableDebugSession', false)
    for (const [key, value] of Object.entries(flatten(config.toJSON()))) {
      factory.setOption(key, value)
    }
    factory.networkInterface = ifs
    for (const layer of linkLayers) {
      factory.registerLinkLayer(layer)
    }
    for (const diss of dissectors) {
      factory.registerDissector(diss)
    }
    for (const macro of filterMacros) {
      factory.registerFilterMacro(macro)
    }
    for (const trans of filterTransforms) {
      factory.registerFilterTransform(trans)
    }
    for (const imp of importers) {
      factory.registerImporter(imp.importer)
    }
    for (const exp of exporters) {
      factory.registerExporter(exp.exporter)
    }
    const attributes = {}
    for (const [key, value] of tokens.entries()) {
      attributes[key] = value
    }
    factory.registerAttributes(attributes)
    return factory.create()
  }

  async runSampleTesting (sample) {
    const asserts = jsonfile.readFileSync(sample.assert)
    const sess = await this.create()
    const prom = (new Promise((res) => {
      sess.on('frame', (stat) => {
        if (stat.frames >= asserts.length) {
          const results = []
          const frames = sess.getFrames(0, asserts.length)
          for (let index = 0; index < frames.length; index += 1) {
            const assertionResults = asserts[index].map((assert) => {
              const filter = sess.createFilter(assert)
              return {
                filter: assert,
                match: Boolean(filter.test(frames[index])),
              }
            })
            results.push({
              frame: frames[index],
              assert: assertionResults,
            })
          }
          res({
            sample,
            results,
          })
        }
      })
    }))
    sess.importFile(sample.pcap)
    return prom
  }

  async runSampleTestingAll () {
    const { samples } = this[fields]
    return Promise.all(Array.from(samples).map((sample) =>
      this.runSampleTesting(sample)
    ))
  }
}
