const m = require('mithril')
const genet = require('@genet/api')
class Enum {
  view (vnode) {
    const { value } = vnode.attrs.attr
    const flags = vnode.attrs.layer.attrs
      .filter((attr) => attr.id.startsWith(`${vnode.attrs.attr.id}.`))
      .map((attr) => {
        const { id } = attr
        return genet.session.tokenName(id)
      })
      .join(', ')
    return m('span', [flags, ' (', value, ')'])
  }
}
module.exports = Enum
