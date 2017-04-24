import Component from './base'
import Session from '../session'
import objpath from 'object-path'
import path from 'path'

export default class StreamDissectorComponent extends Component {
  async load () {
    const main = objpath.get(this.comp, 'stream_dissector.main', '')
    if (main === '') {
      throw new Error('stream_dissector.main field required')
    }
    const mainFile = path.join(this.rootDir, main)
    Session.registerStreamDissector(mainFile)
  }
}
