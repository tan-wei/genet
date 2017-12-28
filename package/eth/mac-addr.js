const m = require('mithril')
class MACAddr {
  view (vnode) {
    const { value } = vnode.attrs.attr
    const addr = value.toJSON().data
      .map((byte) => (`0${byte.toString(16)}`)
      .slice(-2))
      .join(':')
    return m('span', [addr])
  }
}
module.exports = MACAddr
