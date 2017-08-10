import m from 'mithril'

export default class MACAddr {
  constructor() {

  }

  view(vnode) {
    const value = vnode.attrs.prop.value
    const addr = value.map((byte) => {
      return ("0" + byte.toString(16)).slice(-2)
    }).join(':')
    return <span>{ addr }</span>
  }
}
