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

    const name = objpath.get(this.comp, 'listener.name', '')
    if (name === '') {
      throw new Error('listener.name field required')
    }

    const descriptors = objpath.get(
      this.comp, 'listener.descriptors', {})
    Session.addDescriptors(descriptors)

    const mainFile = path.join(this.rootDir, main)
    if (path.extname(mainFile) === '.node') {
      Session.registerNativeListener(name, mainFile)
    } else {
      Session.registerListener(name, mainFile)
    }
  }
}
