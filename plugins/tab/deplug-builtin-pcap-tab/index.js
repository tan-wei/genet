import m from 'mithril'
import { Panel, Plugin } from 'deplug'
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
          onclick={ ()=>{ location.hash = "!/pcap" } }
        ></input>
      </div>
  }
}

class PcapView {
  constructor() {
    Plugin.loadComponents('dissector')
    console.log('plugin')
  }

  oncreate(vnode) {
    Panel.registerSlot('core:pcap:top', (comp) => {
      m.mount(vnode.dom.querySelector('#pcap-top'), comp)
    })
    Panel.registerSlot('core:pcap:bottom', (comp) => {
      m.mount(vnode.dom.querySelector('#pcap-bottom'), comp)
    })

    this.topDragArea = vnode.dom.querySelector('#pcap-top-drag-area')
  }

  startDrag() {
    this.topDragArea.style.visibility = 'visible'
  }

  endDrag() {
    this.topDragArea.style.visibility = 'hidden'
  }

  move(event) {
    console.log(event.clientY)
  }

  view(vnode) {
    let bottomHeight = 200
    return <div>
      <div id="pcap-top"
        style={{bottom: `${bottomHeight}px`}}
        ></div>
      <div class="vertical-handle"
        style={{bottom: `${bottomHeight}px`}}
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
      <div id="pcap-bottom"
        style={{height: `${bottomHeight}px`}}
        ></div>
    </div>
  }
}

export default {
  '/pcap': ConfigView,
  '/': PcapView
}
