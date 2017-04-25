import m from 'mithril'
import { Panel, Plugin, Theme, Tab } from 'deplug'
import { Pcap } from 'plugkit'

class ConfigView {
  constructor() {

  }

  view(vnode) {
    return <div>
        <h1>Live capture</h1>
        <p>
          Permission: { Pcap.permission ? 'OK' : 'NG'}
        </p>
        <ul>
          {
            Pcap.devices.map((dev) => {
              return <li>{ dev.name }</li>
            })
          }
        </ul>
        <h1>Import local file</h1>
        <input
          type="button"
          value="Choose pcap file..."
          onclick={ ()=>{ Tab.page = 'pcap' } }
        ></input>
      </div>
  }
}

class PcapView {
  constructor() {
    Plugin.loadComponents('dissector')
    Plugin.loadComponents('stream-dissector')
    this.bottomHeight = 200
  }

  oncreate(vnode) {
    this.top = vnode.dom.querySelector('#pcap-top').attachShadow({mode: 'open'})
    this.bottom = vnode.dom.querySelector('#pcap-bottom').attachShadow({mode: 'open'})
    this.tool = vnode.dom.querySelector('#pcap-tool').attachShadow({mode: 'open'})
    Panel.registerSlot('core:pcap:top', (comp, less) => {
      Theme.current.render(less).then((style) => {
        const styleTag = document.createElement('style')
        styleTag.textContent = style.css
        m.mount(this.top, comp)
        this.top.append(styleTag)
      })
    })
    Panel.registerSlot('core:pcap:bottom', (comp, less) => {
      Theme.current.render(less).then((style) => {
        const styleTag = document.createElement('style')
        styleTag.textContent = style.css
        m.mount(this.bottom, comp)
        this.bottom.append(styleTag)
      })
    })
    Panel.registerSlot('core:pcap:tool', (comp, less) => {
      Theme.current.render(less).then((style) => {
        const styleTag = document.createElement('style')
        styleTag.textContent = style.css
        m.mount(this.tool, comp)
        this.tool.append(styleTag)
      })
    })
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
        ></div>
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
        <div id="pcap-tool"></div>
        <div id="pcap-bottom"></div>
      </div>
    </div>
  }
}

export default class View {
  view(vnode) {
    if (Tab.page === '') {
      return m(PcapView, {})
    }
    return m(ConfigView, {})
  }
}
