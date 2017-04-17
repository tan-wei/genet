import DissectorComponent from './dissector'
import PanelComponent from './panel'
import TabComponent from './tab'
import ThemeComponent from './theme'
import WindowComponent from './window'

export default class ComponentFactory {
  static create (rootDir, parc, comp) {
    switch (comp.type) {
      case 'panel':
        return new PanelComponent(rootDir, parc, comp)
      case 'tab':
        return new TabComponent(rootDir, parc, comp)
      case 'theme':
        return new ThemeComponent(rootDir, parc, comp)
      case 'window':
        return new WindowComponent(rootDir, parc, comp)
      case 'dissector':
        return new DissectorComponent(rootDir, parc, comp)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }
}
