import {
  GlobalChannel,
  Panel,
  Theme
} from 'deplug'
import m from 'mithril'
class PanelSlot {
  constructor() {
    GlobalChannel.on('core:theme:updated', () => {
      this.updateTheme()
    })
    this.ready = false
  }
  oncreate(vnode) {
    this.panel = vnode.attrs.panel
    const div = document.createElement('div')
    div.classList.add('slot-wrapper')
    const node = div.attachShadow({
      mode: 'open'
    })
    m.mount(node, this.panel.component)
    vnode.dom.parentNode.appendChild(div)
    this.node = node
    this.updateTheme()
    m.redraw()
  }
  updateTheme() {
    Theme.current.render(this.panel.less).then((style) => {
      const styleTag = document.createElement('style')
      styleTag.classList.add('slot-style')
      styleTag.textContent = style.css
      const old = this.node.querySelector('.slot-style')
      if (old) {
        old.remove()
      }
      this.node.append(styleTag)
      this.ready = true
      m.redraw()
    })
  }
  onupdate(vnode) {
    if (this.node) {
      if (this.ready && vnode.attrs.active) {
        this.node.host.style.cssText = 'visibility: visible;'
      } else {
        this.node.host.style.cssText = 'visibility: hidden;'
      }
    }
  }
  view(vnode) {
    return m('div')
  }
}
class MultiPanelSlot {
  constructor() {
    this.panels = []
  }
  oncreate(vnode) {
    this.panels = Panel.get(vnode.attrs.slot)
  }
  view(vnode) {
    return this.panels.map((panel) => {
      return m(PanelSlot, {
        panel,
        active: true
      })
    })
  }
}
class TabSlot {
  constructor() {
    this.panels = []
    this.currentIndex = 0
  }
  oncreate(vnode) {
    this.panels = Panel.get(vnode.attrs.slot)
    m.redraw()
  }
  activate(index) {
    this.currentIndex = parseInt(index)
  }
  view(vnode) {
    return m('div', {
      class: 'tab-container'
    }, [
      m('div', {
        class: 'tab-header'
      }, [
        this.panels.map((panel, index) => {
          return m('a', {
            class: 'tab-label',
            onclick: () => this.activate(index),
            isactive: this.currentIndex === index
          }, [
            panel.name
          ])
        })
      ]),
      this.panels.map((panel, index) => {
        return m(PanelSlot, {
          panel,
          active: this.currentIndex === index
        })
      })
    ])
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
    return m('div', {
      id: 'pcap-separator-wrap'
    }, [
      m('div', {
        class: 'vertical-handle',
        style: {
          top: `${this.separatorRatio * 100}%`,
          transform: 'translateY(-4px)'
        },
        onmousedown: (event) => {
          this.startDrag(event)
        },
        onmouseup: (event) => {
          this.endDrag(event)
        }
      }),
      m('div', {
        id: 'pcap-separator-drag-area',
        class: 'vertical-drag-area',
        onmouseup: (event) => {
          this.endDrag(event)
        },
        onmouseout: (event) => {
          this.endDrag(event)
        },
        onmousemove: (event) => {
          this.move(event)
        }
      }),
      m('div', {
        id: 'pcap-middle',
        style: {
          bottom: `${(1 - this.separatorRatio) * 100}%`
        }
      }, [
        m(TabSlot, {
          slot: 'core:pcap:middle'
        })
      ]),
      m('div', {
        id: 'pcap-bottom',
        style: {
          top: `${this.separatorRatio * 100}%`
        }
      }, [
        m(TabSlot, {
          slot: 'core:pcap:bottom'
        })
      ])
    ])
  }
}
export default class PcapView {
  constructor() {
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
    this.bottomHeight = Math.min(maxBottomHeight, Math.max(this.bottomHeight, minBottomHeight))
  }
  view(vnode) {
    return m('div', [
      m('div', {
        id: 'pcap-top',
        style: {
          bottom: `${this.bottomHeight}px`
        }
      }, [
        m(MultiPanelSlot, {
          slot: 'core:pcap:top'
        })
      ]),
      m('div', {
        class: 'vertical-handle',
        style: {
          bottom: `${this.bottomHeight}px`
        },
        onmousedown: (event) => {
          this.startDrag(event)
        },
        onmouseup: (event) => {
          this.endDrag(event)
        }
      }),
      m('div', {
        id: 'pcap-top-drag-area',
        class: 'vertical-drag-area',
        onmouseup: (event) => {
          this.endDrag(event)
        },
        onmouseout: (event) => {
          this.endDrag(event)
        },
        onmousemove: (event) => {
          this.move(event)
        }
      }),
      m('div', {
        id: 'pcap-drawer-wrap',
        style: {
          height: `${this.bottomHeight}px`
        }
      }, [
        m('div', {
          id: 'pcap-tool'
        }, [
          m(MultiPanelSlot, {
            slot: 'core:pcap:tool'
          })
        ]),
        m(DrawerView, {})
      ])
    ])
  }
}
