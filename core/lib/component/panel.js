import BaseComponent from './base'
import Script from '../script'
import Style from '../style'
import objpath from 'object-path'
import path from 'path'

export default class PanelComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }
    this.id = objpath.get(comp, 'id', '')
    if (!this.id) {
      throw new Error('id field required')
    }
    this.mainFile = path.resolve(dir, file)
    const style = objpath.get(comp, 'style', '')
    if (style.length > 0) {
      this.styleFile = path.resolve(dir, style)
    }
  }

  async load () {
    let style = ''
    if (this.styleFile) {
      const loader = new Style()
      const result = await loader.compileLess(this.styleFile)
      style = result.css
    }
    const component = await Script.execute(this.mainFile)
    this.disposable =
      deplug.workspace.registerPanel(this.id, {
        component,
        style,
      })
    return true
  }

  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
      this.disposable = null
    }
    return true
  }
}
