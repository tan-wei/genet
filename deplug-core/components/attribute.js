import Component from './base'
import Session from '../session'
import objpath from 'object-path'

export default class AttributeComponent extends Component {
  async load () {
    const attribute = objpath.get(this.comp, 'attribute', {})
    Session.addAttributes(attribute)
  }
}
