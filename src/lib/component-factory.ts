import { ActionComponent } from './component/action'
import { FileComponent } from './component/file'
import { LibraryComponent } from './component/library'
import { MacroComponent } from './component/macro'
import { PanelComponent } from './component/panel'
import { RendererComponent } from './component/renderer'
import { StyleComponent } from './component/style'
import { TokenComponent } from './component/token'
export default class ComponentFactory {
  static create(comp: any, dir: string) {
    switch (comp.type) {
      case 'core:token':
        return new TokenComponent.Loader(comp, dir)
      case 'core:action':
        return new ActionComponent.Loader(comp, dir)
      case 'core:library':
        return new LibraryComponent.Loader(comp, dir)
      case 'core:file:reader':
        return new FileComponent.Loader(comp, dir)
      case 'core:renderer:attr':
      case 'core:renderer:layer':
        return new RendererComponent.Loader(comp, dir)
      case 'core:style':
        return new StyleComponent.Loader(comp, dir)
      case 'core:panel':
        return new PanelComponent.Loader(comp, dir)
      case 'core:filter:macro':
        return new MacroComponent.Loader(comp, dir)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }
}
