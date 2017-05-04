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

class DrawerView {
  constructor() {
    this.separatorRatio = 0.5
  }

  oncreate(vnode) {
    this.dom = vnode.dom
    this.dragArea = vnode.dom.querySelector('#pcap-separator-drag-area')
  }

  startDrag(event) {
    this.dragArea.style.visibility = 'visible'
    if (!this.dragArea.hasAttribute('data-start-y')) {
      this.dragArea.setAttribute('data-start-y', event.clientY)
    }
  }

  endDrag() {
    this.dragArea.style.visibility = 'hidden'
    this.dragArea.removeAttribute('data-start-y')
  }

  move(event) {
    this.separatorRatio = event.offsetY / this.dragArea.clientHeight
  }

  view(vnode) {
    return <div id="pcap-separator-wrap">
      <div class="vertical-handle"
        style={{
          top: `${this.separatorRatio * 100}%`,
          transform: 'translateY(-4px)'
        }}
        onmousedown={(event) => {this.startDrag(event)}}
        onmouseup={(event) => {this.endDrag(event)}}
        ></div>
      <div
        id="pcap-separator-drag-area"
        class="vertical-drag-area"
        onmouseup={(event) => {this.endDrag(event)}}
        onmouseout={(event) => {this.endDrag(event)}}
        onmousemove={(event) => {this.move(event)}}
        ></div>
      <div id="pcap-middle"
      style={{bottom: `${(1 - this.separatorRatio) * 100}%`}}
      >{
        m(PanelSlot, {slot: 'core:pcap:middle'})
      }</div>
      <div id="pcap-bottom"
      style={{top: `${this.separatorRatio * 100}%`}}
      >{
        m(PanelSlot, {slot: 'core:pcap:middle'})
      }</div>
    </div>
  }
}

export default class PcapView {
  constructor() {
    Plugin.loadComponents('dissector')
    Plugin.loadComponents('stream-dissector')
    this.bottomHeight = 300
  }

  oncreate(vnode) {
    this.dragArea = vnode.dom.querySelector('#pcap-top-drag-area')
  }

  startDrag(event) {
    this.dragArea.style.visibility = 'visible'
    if (!this.dragArea.hasAttribute('data-start-y')) {
      this.dragArea.setAttribute('data-start-y', event.clientY)
    }
  }

  endDrag() {
    this.dragArea.style.visibility = 'hidden'
    this.dragArea.removeAttribute('data-start-y')
  }

  move(event) {
    const minBottomHeight = 32
    const maxBottomHeight = this.dragArea.clientHeight - minBottomHeight
    this.bottomHeight = this.dragArea.clientHeight - event.clientY
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
        onmouseup={(event) => {this.endDrag(event)}}
        onmouseout={(event) => {this.endDrag(event)}}
        onmousemove={(event) => {this.move(event)}}
        ></div>
      <div id="pcap-drawer-wrap"
        style={{height: `${this.bottomHeight}px`}}
      >
      <div id="pcap-tool">{
        m(PanelSlot, {slot: 'core:pcap:tool'})
      }</div>
      {
        m(DrawerView, {})
      }
      </div>
    </div>
  }
}
