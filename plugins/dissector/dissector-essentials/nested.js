import m from 'mithril'
import {Session} from 'deplug'

export default class Nested {
  view(vnode) {
    const keys = vnode.attrs.layer.properties.filter((prop) => {
      return prop.id.startsWith(vnode.attrs.prop.id + '.')
    }).map((prop) => {
      const id = prop.id
      const name = (id in Session.attributes) ? Session.attributes[id].name : id
      return name
    }).join(', ')
    return <span>{ keys }</span>
  }
}
