import Component from './base'
import Session from '../session'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'

export default class DissectorComponent extends Component {
  async load () {
    const type = objpath.get(this.comp, 'dissector.type', '')
    if (type === '') {
      throw new Error('dissector.type field required')
    }

    const main = objpath.get(this.comp, 'dissector.main', '')
    if (main === '') {
      throw new Error('dissector.main field required')
    }
    const mainFile = path.join(this.rootDir, main)
    switch (type) {
      case 'packet':
        if (path.extname(mainFile) === '.node') {
          Session.registerNativeDissector(mainFile)
        } else {
          Session.registerDissector(mainFile)
        }
        break
      case 'stream':
        if (path.extname(mainFile) === '.node') {
          Session.registerNativeStreamDissector(mainFile)
        } else {
          Session.registerStreamDissector(mainFile)
        }
        break
      default:
        throw new Error('unknown dissector.type')
    }

    const linkLayers = objpath.get(this.comp, 'dissector.linkLayers', [])
    for (const layer of linkLayers) {
      Session.registerLinkLayer(layer)
    }

    const descriptors = objpath.get(this.comp, 'dissector.descriptors', {})
    Session.addDescriptors(descriptors)

    const samples = objpath.get(this.comp, 'dissector.samples', [])
    for (const item of samples) {
      const sample = { pcap: path.join(this.rootDir, item.pcap) }
      if (item.assert) {
        sample.assert =
          jsonfile.readFileSync(path.join(this.rootDir, item.assert))
      }
      Session.addSample(sample)
    }
  }
}
