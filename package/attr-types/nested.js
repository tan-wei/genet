const m = require('mithril')
class Nested {
  view (vnode) {
    const keys = vnode.attrs.layer.attrs
      .filter((prop) => prop.id.startsWith(`${vnode.attrs.prop.id}.`))
      .map((prop) => {
        const { id } = prop
        return deplug.session.token(id).name
    })
    .join(', ')
    return m('span', [keys])
  }
}
module.exports = Nested
