import m from 'mithril'

export default class IPv4Addr {
  view(vnode) {
    const value = vnode.attrs.prop.value
    const addr = [
      (value >> 24) & 0xff,
      (value >> 16) & 0xff,
      (value >> 8) & 0xff,
      value & 0xff
    ]
    return <span>{addr[0]}.{addr[1]}.{addr[2]}.{addr[3]}</span>
  }
}
