import m from 'mithril'
export default class MACAddr {
  view(vnode) {
    const value = vnode.attrs.prop.value
    const addr = value.toJSON().data.map((byte) => (`0${byte.toString(16)}`).slice(-2)).join(':')
    return m('span', [addr])
  }
}
