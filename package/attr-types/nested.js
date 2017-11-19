import { Session } from 'deplug'
import m from 'mithril'

export default class Nested {
  view (vnode) {
    const keys = vnode.attrs.layer.attrs
      .filter((prop) => prop.id.startsWith(`${vnode.attrs.prop.id}.`))
      .map((prop) => {
        const { id } = prop
        return (id in Session.attributes)
          ? Session.attributes[id].name
          : id
    })
    .join(', ')
    return m('span', [keys])
  }
}
