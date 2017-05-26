import Component from './base'
import Session from '../session'
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

    const linkLyaers = objpath.get(this.comp, 'dissector.linkLyaers', [])
    for (const layer of linkLyaers) {
      Session.registerLinkLayer(layer)
    }

    const samples = objpath.get(this.comp, 'dissector.samples', [])
    for (const samp of samples) {
      Session.addSample({ pcap: path.join(this.rootDir, samp.pcap) })
    }
  }
}
