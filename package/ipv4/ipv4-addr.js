const m = require('mithril')
class IPv4Addr {
  view (vnode) {
    const { value } = vnode.attrs.prop
    return m('span', [value[0], '.', value[1], '.', value[2], '.', value[3]])
  }
}
module.exports = IPv4Addr
