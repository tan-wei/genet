import m from 'mithril'
import {Token} from 'plugkit'
import {Session} from 'deplug'

export default class Nested {
  constructor() {

  }

  view(vnode) {
    const properties = vnode.attrs.prop.properties
    return <span>{
      properties.filter((prop)=>{
        return prop.value
      }).map((prop) => {
        const str = prop.id
        const path = vnode.attrs.path + '.' + str
        const name = (path in Session.descriptors) ?
          Session.descriptors[path].name : str
        return name
      }).join(', ')
    }</span>
  }
}
