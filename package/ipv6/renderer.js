const m = require('mithril')
const addr = require('./ipv6-addr')
class IPv6Addr {
  view (vnode) {
    const { value } = vnode.attrs.attr
    return m('span', [addr(value)])
  }
}
module.exports = IPv6Addr
