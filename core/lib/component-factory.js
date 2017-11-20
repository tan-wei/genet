import BaseComponent from './component/base'
import DissectorComponent from './component/dissector'
import TokenComponent from './component/token'
export default class ComponentFactory {
 static create (comp, dir) {
    switch (comp.type) {
      case 'core:token':
        return new TokenComponent(comp, dir)
      case 'core:dissector:packet':
      case 'core:dissector:stream':
        return new DissectorComponent(comp, dir)
      case 'core:renderer:attr':
      case 'core:filter:transform':
      default:
        return new BaseComponent(comp, dir)
    }
  }
}
