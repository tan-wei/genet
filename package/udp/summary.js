const m = require('mithril')
class Summary {
  view (vnode) {
    const { layer } = vnode.attrs
    const { parent } = layer
    const srcAddr = parent.attr(`${parent.id}.src`)
    const dstAddr = parent.attr(`${parent.id}.dst`)
    const srcPort = layer.attr(`${layer.id}.src`).getValue()
    const dstPort = layer.attr(`${layer.id}.dst`).getValue()
    const renderer = genet.session.attrRenderer(srcAddr.type)
    if (srcAddr.type === '@ipv6:addr') {
      return m('span', [
        ' [', m(renderer, { attr: srcAddr }), ']:',
        m('b', [srcPort]), ' -> [', m(renderer, { attr: dstAddr }), ']:',
        m('b', [dstPort])
      ])
    }
    return m('span', [' ',
      m(renderer, { attr: srcAddr }), ':', m('b', [srcPort]),
      ' ->', m(renderer, { attr: dstAddr }), ':', m('b', [dstPort])
    ])
  }
}
module.exports = Summary
