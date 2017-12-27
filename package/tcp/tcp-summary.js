const m = require('mithril')
class TCPSummary {
  view (vnode) {
    const { layer } = vnode.attrs
    const { parent } = layer
    const srcAddr = parent.attr(`${parent.id}.src`)
    const dstAddr = parent.attr(`${parent.id}.dst`)
    const srcPort = layer.attr(`${layer.id}.src`).value
    const dstPort = layer.attr(`${layer.id}.dst`).value
    const renderer = deplug.session.attrRenderer(srcAddr.type)
    if (srcAddr.type === '@ipv6:addr') {
      return m('span', [
        ' [',
        m(renderer, { prop: srcAddr }),
        ']:',
        m('b', [srcPort]),
        ' -> [',
         m(renderer, { prop: dstAddr }),
         ']:',
         m('b', [dstPort]),
         ' [', layer.payloads.length, '] '
       ])
    }
    return m('span', [
      ' ',
      m(renderer, { prop: srcAddr }),
      ':',
      m('b', [srcPort]),
      ' ->',
      m(renderer, { prop: dstAddr }),
      ':',
      m('b', [dstPort]),
      ' [',
      layer.payloads.length,
      '] '
    ])
  }
}
module.exports = TCPSummary
