import BaseComponent from './base'
import { CompositeDisposable } from 'disposables'
import { _load } from 'module'
import exists from 'file-exists'
import objpath from 'object-path'
import path from 'path'

export default class DissectorComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }

    const searchPaths = [
      '.',
      'build/Debug',
      'build/Release'
    ]
    for (const spath of searchPaths) {
      const absolute = path.join(dir, spath, file)
      if (exists.sync(absolute)) {
        this.mainFile = absolute
        break
      }
    }
    if (!this.mainFile) {
      throw new Error(`could not resolve ${file} in ${dir}`)
    }

    this.linkLayers = objpath.get(comp, 'linkLayers', [])

    this.samples = objpath.get(comp, 'samples', [])
      .map((sample) => ({
        pcap: path.join(dir, sample.pcap),
        assert: path.join(dir, sample.assert),
      }))

    switch (comp.type) {
      case 'core:dissector:packet':
        this.type = 'packet'
        break
      case 'core:dissector:stream':
        this.type = 'stream'
        break
      default:
        throw new Error(`unknown dissector type: ${comp.type}`)
    }
  }
  async load () {
    const ext = path.extname(this.mainFile)
    switch (ext) {
      case '.node':
      case '.js':
        this.disposable = deplug.session.registerDissector({
          type: this.type,
          main: this.mainFile,
        })
        break
      default:
        throw new Error(`unknown extension type: ${ext}`)
    }
    for (const layer of this.linkLayers) {
      this.disposable = new CompositeDisposable([
        this.disposable,
        deplug.session.registerLinkLayer(layer)
      ])
    }
    for (const sample of this.samples) {
      this.disposable = new CompositeDisposable([
        this.disposable,
        deplug.session.registerSample(sample)
      ])
    }
    return false
  }
  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
      this.disposable = null
      return false
    }
    return true
  }
}
