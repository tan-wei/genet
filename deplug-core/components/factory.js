import ThemeComponent from './theme'
import WindowComponent from './window'

export default class ComponentFactory {
  static create (rootPath, comp) {
    switch (comp.type) {
      case 'theme':
        return new ThemeComponent(comp.type, rootPath, comp)
      case 'window':
        return new WindowComponent(comp.type, rootPath, comp)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }
}
