import DissectorComponent from './component/dissector'
import RendererComponent from './component/renderer'
import StyleComponent from './component/style'
import TokenComponent from './component/token'
import TransformComponent from './component/transform'
export default class ComponentFactory {
 static create (comp, dir) {
    switch (comp.type) {
      case 'core:token':
        return new TokenComponent(comp, dir)
      case 'core:dissector:packet':
      case 'core:dissector:stream':
        return new DissectorComponent(comp, dir)
      case 'core:renderer:attr':
      case 'core:renderer:layer':
        return new RendererComponent(comp, dir)
      case 'core:style':
        return new StyleComponent(comp, dir)
      case 'core:filter:transform':
        return new TransformComponent(comp, dir)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }
}
