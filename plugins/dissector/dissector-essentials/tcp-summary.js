import m from 'mithril'
import { Renderer } from 'deplug';

export default class TCPSummary {
  view(vnode) {
    const layer = vnode.attrs.layer
    const parent = layer.parent
    const srcAddr = parent.propertyFromId(parent.id + '.src')
    const dstAddr = parent.propertyFromId(parent.id + '.dst')
    const srcPort = layer.propertyFromId('tcp.src').value
    const dstPort = layer.propertyFromId('tcp.dst').value
    const renderer = Renderer.forProperty(srcAddr.type)
    if (srcAddr.type === '@ipv6:addr') {
      return <span> [{ m(renderer, {prop: srcAddr}) }]:<b>{ srcPort }</b> ->
        [{ m(renderer, {prop: dstAddr}) }]:<b>{ dstPort }</b> </span>
    }
    return <span> { m(renderer, {prop: srcAddr}) }:<b>{ srcPort }</b> ->
      { m(renderer, {prop: dstAddr}) }:<b>{ dstPort }</b> </span>
  }
}
