import m from 'mithril'
import { Renderer } from 'deplug';

export default class TCPSummary {
  view(vnode) {
    const layer = vnode.attrs.layer
    const parent = layer.parent
    const srcAddr = parent.propertyFromId('.src')
    const dstAddr = parent.propertyFromId('.dst')
    const srcPort = layer.propertyFromId('.src').value
    const dstPort = layer.propertyFromId('.dst').value
    const renderer = Renderer.forProperty(srcAddr.type)
    if (srcAddr.type === '@ipv6:addr') {
      return <span> [{ m(renderer, {prop: srcAddr}) }]:<b>{ srcPort }</b> ->
        [{ m(renderer, {prop: dstAddr}) }]:<b>{ dstPort }</b> </span>
    }
    return <span> { m(renderer, {prop: srcAddr}) }:<b>{ srcPort }</b> ->
      { m(renderer, {prop: dstAddr}) }:<b>{ dstPort }</b> </span>
  }
}
