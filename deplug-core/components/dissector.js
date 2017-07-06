import Component from './base'
import Session from '../session'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'

export default class DissectorComponent extends Component {
  async load () {
    const main = objpath.get(this.comp, 'dissector.main', '')
    if (main === '') {
      throw new Error('dissector.main field required')
    }
    const mainFile = path.join(this.rootDir, main)
    if (path.extname(mainFile) === '.node') {
      Session.registerNativeDissector(mainFile)
    } else {
      Session.registerDissector(mainFile)
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
