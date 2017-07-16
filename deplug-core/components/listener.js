import Component from './base'
import Session from '../session'
import objpath from 'object-path'
import path from 'path'

export default class ListenerComponent extends Component {
  async load () {
    const main = objpath.get(this.comp, 'listener.main', '')
    if (main === '') {
      throw new Error('listener.main field required')
    }

    const id = objpath.get(this.comp, 'listener.id', '')
    if (id === '') {
      throw new Error('listener.id field required')
    }

    const descriptors = objpath.get(
      this.comp, 'listener.descriptors', {})
    Session.addDescriptors(descriptors)

    const mainFile = path.join(this.rootDir, main)
    if (path.extname(mainFile) === '.node') {
      Session.registerNativeListener(id, mainFile)
    } else {
      Session.registerListener(id, mainFile)
    }
  }
}
