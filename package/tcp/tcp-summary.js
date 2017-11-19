import { Renderer } from 'deplug'
import m from 'mithril'

export default class TCPSummary {
  view (vnode) {
    const layer = vnode.attrs.prop.value
    const { parent } = layer
    const srcAddr = parent.attr('.src')
    const dstAddr = parent.attr('.dst')
    const srcPort = layer.attr('.src').value
    const dstPort = layer.attr('.dst').value
    const renderer = Renderer.forProperty(srcAddr.type)
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
