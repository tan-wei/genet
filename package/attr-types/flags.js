const m = require('mithril')
class Flags {
  view (vnode) {
    const value = vnode.attrs.attr.getValue()
    const flags = vnode.attrs.layer.attrs
      .filter((attr) => attr.getValue() && attr.id
        .startsWith(`${vnode.attrs.attr.id}.`))
      .map((attr) => {
        const { id } = attr
        return genet.session.tokenName(id)
      })
      .join(', ')
    return m('span', [flags, ' (', value, ')'])
  }
}
module.exports = Flags
