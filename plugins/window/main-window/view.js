import {
  ipcRenderer,
  remote
} from 'electron'
import {
  Argv,
  GlobalChannel,
  Tab,
  Menu,
  Profile
} from 'deplug'
import m from 'mithril'
import url from 'url'
import i18n from 'i18n4v'
const {
  MenuItem
} = remote
class WebView {
  onupdate(vnode) {
    if (vnode.attrs.isLoaded && typeof this.guestinstance === 'undefined') {
      this.guestinstance = vnode.dom.getAttribute('guestinstance')
    }
  }
  view(vnode) {
    const item = vnode.attrs.item
    const src = url.format({
      protocol: 'file',
      pathname: `${__dirname}/index.htm`,
    })
    return m('webview', {
      id: `tab-content-${vnode.attrs.key}`,
      class: 'tab-content',
      src,
      isActive: vnode.attrs.isActive,
      isLoaded: vnode.attrs.isLoaded,
      useragent: JSON.stringify([Profile.currentId, Argv, item]),
      preload: `file://${__dirname}/preload.js`,
    })
  }
}
export default class TabView {
  constructor() {
    this.idCounter = 0
    this.currentIndex = 0
    this.loadedTabs = []
    this.tabs = []
    GlobalChannel.on('core:tab:open', (_, template, options = {}) => {
      const tab = Tab.getTemplate(template)
      if (tab.singleton === true) {
        const index = this.tabs.findIndex((t) => t.tab.template === template)
        if (index >= 0) {
          this.currentIndex = index
          m.redraw()
          return
        }
      }
      this.tabs.push({
        id: this.idCounter++,
        tab: Object.assign({}, tab),
        options,
      })
      this.currentIndex = this.tabs.length - 1
      m.redraw()
    })
    GlobalChannel.on('core:tab:set-name', (_, id, name) => {
      const item = this.tabs.find((t) => t.id === id)
      if (item) {
        item.tab.name = name
        m.redraw()
      }
    })
    GlobalChannel.on('core:tab:focus', (_, id) => {
      const index = this.tabs.findIndex((t) => t.id === id)
      if (index >= 0) {
        this.currentIndex = index
        m.redraw()
      }
    })
    GlobalChannel.on('core:tab:close', (_, id) => {
      const index = this.tabs.findIndex((t) => t.id === id)
      if (index >= 0) {
        if (this.currentIndex > 0 && this.currentIndex === this.tabs.length - 1) {
          this.currentIndex--
        }
        const content = document.querySelector(`#tab-content-${index}`)
        if (content) {
          content.closeDevTools()
        }
        this.tabs.splice(index, 1)
        if (this.tabs.length === 0) {
          remote.getCurrentWindow().close()
        }
        m.redraw()
      }
    })
    ipcRenderer.on('tab-deplug-loaded', (_, id) => {
      this.loadedTabs[id] = true
      m.redraw()
    })
    Menu.register('core:tab:context', this.tabMenu)
  }
  oncreate(vnode) {
    this.tabContainer = vnode.dom.querySelector('#tab-container')
    this.styleTag = document.createElement('style')
    document.head.appendChild(this.styleTag)
  }
  activate(index) {
    this.currentIndex = parseInt(index)
  }
  tabMenu(menu, e) {
    menu.append(new MenuItem({
      label: 'Show Developer Tools',
      click: () => {
        const id = e.target.getAttribute('tabId')
        const content = document.querySelector(`#tab-content-${id}`)
        content.openDevTools()
      },
    }))
    menu.append(new MenuItem({
      label: 'Reload',
      click: () => {
        const id = e.target.getAttribute('tabId')
        const content = document.querySelector(`#tab-content-${id}`)
        content.reload()
      },
    }))
    return menu
  }
  dragStart(event) {
    event.target.style.opacity = '0.4'
    event.dataTransfer.effectAllowed = 'move'
    event.dataTransfer.setData('text/plain', event.target.getAttribute('index'))
    this.styleTag.textContent = 'webview { pointer-events: none; }'
  }
  dragEnd(event) {
    event.target.style.opacity = '1.0'
    this.styleTag.textContent = ''
  }
  dragDrop(event) {
    const currentIndex = parseInt(event.dataTransfer.getData('text/plain'))
    const index = parseInt(event.target.getAttribute('index'))
    if (index !== currentIndex) {
      const id = this.tabs[this.currentIndex].id
      const tmp = this.tabs[index]
      this.tabs[index] = this.tabs[currentIndex]
      this.tabs[currentIndex] = tmp
      this.currentIndex = this.tabs.findIndex((t) => t.id === id)
      m.redraw()
    }
    event.target.classList.remove('over')
  }
  dragEnter(event) {
    event.target.classList.add('over')
  }
  dragLeave(event) {
    event.target.classList.remove('over')
  }
  dragOver(event) {
    if (event.preventDefault) {
      event.preventDefault()
    }
    event.dataTransfer.dropEffect = 'move'
    return false
  }
  view() {
    return (m('main', [
      m('div', {
        id: 'tab-container'
      }, [
        this.tabs.map((t, i) => (m('a', {
          class: 'tab-label',
          draggable: 'true',
          index: i,
          tabId: t.id,
          isActive: this.currentIndex === i,
          onclick: m.withAttr('index', this.activate, this),
          ondragstart: (e) => {
            this.dragStart(e)
          },
          ondragend: (e) => {
            this.dragEnd(e)
          },
          ondragenter: (e) => {
            this.dragEnter(e)
          },
          ondragleave: (e) => {
            this.dragLeave(e)
          },
          ondrop: (e) => {
            this.dragDrop(e)
          },
          ondragover: (e) => {
            this.dragOver(e)
          },
          oncontextmenu: (e) => {
            Menu.popup('core:tab:context', this, remote.getCurrentWindow(), {
              event: e
            })
          },
        }, [
          m('i', {
            class: 'fa fa-times',
            onclick: () => {
              if (this.currentIndex === i) {
                GlobalChannel.emit('core:tab:close', t.id)
              }
            },
          }),
          i18n(t.tab.name)
        ])))
      ]),
      m('div', {
        id: 'menu-container'
      }, [
        m('a', {
          class: 'tab-menu-button',
          onclick: () => {
            GlobalChannel.emit('core:tab:open', 'Preferences')
          },
        }, [
          m('i', {
            class: 'fa fa-cog'
          })
        ]),
        m('a', {
          class: 'tab-menu-button',
          onclick: () => {
            GlobalChannel.emit('core:tab:open', 'Pcap')
          },
        }, [
          m('i', {
            class: 'fa fa-plus'
          })
        ])
      ]), (() => {
        const tabs = [].concat(this.tabs)
        tabs.sort((tab1, tab2) => tab1.id - tab2.id)
        return tabs
      })().map((t, i) => m(WebView, {
        key: t.id,
        item: t,
        isLoaded: (this.loadedTabs[t.id] === true),
        isActive: this.tabs[this.currentIndex].id === t.id,
      }))
    ]))
  }
}
