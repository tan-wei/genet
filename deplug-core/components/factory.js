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
  static create (rootDir, pkg, comp) {
    switch (comp.type) {
      case 'panel':
        return new PanelComponent(rootDir, pkg, comp)
      case 'tab':
        return new TabComponent(rootDir, pkg, comp)
      case 'theme':
        return new ThemeComponent(rootDir, pkg, comp)
      case 'window':
        return new WindowComponent(rootDir, pkg, comp)
      case 'dissector':
        return new DissectorComponent(rootDir, pkg, comp)
      case 'stream-dissector':
        return new StreamDissectorComponent(rootDir, pkg, comp)
      case 'menu':
        return new MenuComponent(rootDir, pkg, comp)
      case 'script':
        return new ScriptComponent(rootDir, pkg, comp)
      case 'file':
        return new FileComponent(rootDir, pkg, comp)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }
}
