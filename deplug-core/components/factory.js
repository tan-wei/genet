import DissectorComponent from './dissector'
import FileComponent from './file'
import MenuComponent from './menu'
import PanelComponent from './panel'
import ScriptComponent from './script'
import StreamDissectorComponent from './stream-dissector'
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
      case 'stream-dissector':
        return new StreamDissectorComponent(rootDir, parc, comp)
      case 'menu':
        return new MenuComponent(rootDir, parc, comp)
      case 'script':
        return new ScriptComponent(rootDir, parc, comp)
      case 'file':
        return new FileComponent(rootDir, parc, comp)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }
}
