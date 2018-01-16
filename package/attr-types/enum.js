const m = require('mithril')
class Enum {
  view (vnode) {
    const { value } = vnode.attrs.attr
    const flags = vnode.attrs.layer.attrs
      .filter((attr) => attr.id.startsWith(`${vnode.attrs.attr.id}.`))
      .map((attr) => {
        const { id } = attr
        return deplug.session.token(id).name
      })
      .join(', ')
    return m('span', [flags, ' (', value, ')'])
  }
}
module.exports = Enum
