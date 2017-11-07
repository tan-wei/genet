import BaseComponent from './component/base'
import TokenComponent from './component/token'
export default class ComponentFactory {
 static create (comp, dir) {
    switch (comp.type) {
      case 'token':
        return new TokenComponent(comp, dir)
      default:
        return new BaseComponent(comp, dir)
    }
  }
}
