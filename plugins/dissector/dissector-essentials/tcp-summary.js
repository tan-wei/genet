import m from 'mithril'
import {
  Renderer
} from 'deplug'
export default class TCPSummary {
  view(vnode) {
    const layer = vnode.attrs.prop.value
    const parent = layer.parent
    const srcAddr = parent.attr('.src')
    const dstAddr = parent.attr('.dst')
    const srcPort = layer.attr('.src').value
    const dstPort = layer.attr('.dst').value
    const renderer = Renderer.forProperty(srcAddr.type)
    if (srcAddr.type === '@ipv6:addr') {
      return m('span', [' [', m(renderer, {
        prop: srcAddr
      }), ']:', m('b', [srcPort]), ' ->' + ' ' + '[', m(renderer, {
        prop: dstAddr
      }), ']:', m('b', [dstPort]), ' [', layer.payloads.length, '] '])
    }
    return m('span', [' ', m(renderer, {
        prop: srcAddr
      }), ':', m('b', [srcPort]), ' ->',
      m(renderer, {
        prop: dstAddr
      }), ':', m('b', [dstPort]), ' [', layer.payloads.length, '] '
    ])
  }
}
