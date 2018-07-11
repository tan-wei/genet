import FileComponent from './component/file'
import LibraryComponent from './component/library'
import MacroComponent from './component/macro'
import PanelComponent from './component/panel'
import RendererComponent from './component/renderer'
import StyleComponent from './component/style'
import TokenComponent from './component/token'
export default class ComponentFactory {
  static create (comp, dir) {
    switch (comp.type) {
      case 'core:token':
        return new TokenComponent(comp, dir)
      case 'core:library':
        return new LibraryComponent(comp, dir)
      case 'core:file:reader':
        return new FileComponent(comp, dir)
      case 'core:renderer:attr':
      case 'core:renderer:layer':
        return new RendererComponent(comp, dir)
      case 'core:style':
        return new StyleComponent(comp, dir)
      case 'core:panel':
        return new PanelComponent(comp, dir)
      case 'core:filter:macro':
        return new MacroComponent(comp, dir)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }
}
