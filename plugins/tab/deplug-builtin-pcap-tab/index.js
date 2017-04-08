import m from 'mithril'

class View {
  view(vnode) {
    return <h1><a href="#!/xxxx">Pcap</a></h1>
  }
}

class View2 {
  view(vnode) {
    return <h1><a href="#!/">Pcap Pcap</a></h1>
  }
}

export default {
  '/': View,
  '/xxxx': View2
}
