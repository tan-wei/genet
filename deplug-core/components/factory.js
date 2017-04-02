import ThemeComponent from './theme'
import WindowComponent from './window'

export default class ComponentFactory {
  static create (rootDir, parc, comp) {
    switch (comp.type) {
      case 'theme':
        return new ThemeComponent(rootDir, parc, comp)
      case 'window':
        return new WindowComponent(rootDir, parc, comp)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }
}
