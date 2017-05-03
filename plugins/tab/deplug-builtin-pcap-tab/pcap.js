import { GlobalChannel, Panel, Plugin, Theme } from 'deplug'
import m from 'mithril'

class PanelSlot {
  constructor() {
    GlobalChannel.on('core:theme:updated', () => { this.updateTheme() })
  }

  oncreate(vnode) {
    this.panels = Panel.get(vnode.attrs.slot)
    this.nodes = []
    for (const panel of this.panels) {
      const div = document.createElement('div')
      div.classList.add('slot-wrapper')
      const node = div.attachShadow({mode: 'open'})
      m.mount(node, panel.component)
      vnode.dom.parentNode.appendChild(div)
      node.host.style.cssText = "visibility: hidden";
      this.nodes.push(node)
    }
    this.updateTheme()
  }

  updateTheme() {
    for (let i = 0; i < this.panels.length; ++i) {
      const panel = this.panels[i]
      Theme.current.render(panel.less).then((style) => {
        const styleTag = document.createElement('style')
        styleTag.classList.add('slot-style')
        styleTag.textContent = style.css
        const node = this.nodes[i]
        const old = node.querySelector('.slot-style')
        if (old) {
          old.remove()
        }
        node.append(styleTag)
        node.host.style.cssText = "visibility: visible";
      })
    }
  }

  view(vnode) {
    return <div></div>
  }
}

export default class PcapView {
  constructor() {
    Plugin.loadComponents('dissector')
    Plugin.loadComponents('stream-dissector')
    this.bottomHeight = 200
  }

  oncreate(vnode) {
    this.topDragArea = vnode.dom.querySelector('#pcap-top-drag-area')
  }

  startDrag(event) {
    this.topDragArea.style.visibility = 'visible'
    if (!this.topDragArea.hasAttribute('data-start-y')) {
      this.topDragArea.setAttribute('data-start-y', event.clientY)
    }
  }

  endDrag() {
    this.topDragArea.style.visibility = 'hidden'
    this.topDragArea.removeAttribute('data-start-y')
  }

  move(event) {
    const minBottomHeight = 20
    const maxBottomHeight = this.topDragArea.clientHeight - minBottomHeight
    this.bottomHeight = this.topDragArea.clientHeight - event.clientY
    this.bottomHeight = Math.min(maxBottomHeight,
      Math.max(this.bottomHeight, minBottomHeight))
  }

  view(vnode) {
    return <div>
      <div id="pcap-top"
        style={{bottom: `${this.bottomHeight}px`}}
        >
        {
          m(PanelSlot, {slot: 'core:pcap:top'})
        }
      </div>
      <div class="vertical-handle"
        style={{bottom: `${this.bottomHeight}px`}}
        onmousedown={(event) => {this.startDrag(event)}}
        onmouseup={(event) => {this.endDrag(event)}}
        ></div>
      <div
        id="pcap-top-drag-area"
        class="vertical-drag-area"
        onmousedown={(event) => {this.startDrag(event)}}
        onmouseup={(event) => {this.endDrag(event)}}
        onmouseout={(event) => {this.endDrag(event)}}
        onmousemove={(event) => {this.move(event)}}
        ></div>
      <div id="pcap-bottom-wrap"
        style={{height: `${this.bottomHeight}px`}}
      >
        <div id="pcap-tool">{
          m(PanelSlot, {slot: 'core:pcap:tool'})
        }</div>
        <div id="pcap-bottom">{
          m(PanelSlot, {slot: 'core:pcap:bottom'})
        }</div>
      </div>
    </div>
  }
}
