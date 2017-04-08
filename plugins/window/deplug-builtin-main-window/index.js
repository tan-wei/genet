import { ipcRenderer, remote } from 'electron'
import { Argv, Channel, Tab, Menu } from 'deplug'
import m from 'mithril'
import jquery from 'jquery'
import i18n from 'i18n4v'

const { MenuItem } = remote

class WebView {
  oncreate(vnode) {
    let webview = vnode.dom
    let item = vnode.attrs.item
    if (item) {
      webview.addEventListener('dom-ready', () => {
        const argv = JSON.stringify(Argv)
        const tab = JSON.stringify(item)
        const script = `require("deplug-core/tab.main")(${argv}, ${tab})`
        webview.executeJavaScript(script)
      })
    }
  }

  view(vnode) {
    return <webview
      class="tab-content"
      src={`file://${__dirname}/index.htm`}
      isActive={vnode.attrs.isActive}
      isLoaded={vnode.attrs.isLoaded}
      nodeintegration
    >
    </webview>
  }
}

export default class TabView {
  constructor() {
    this.idCounter = 0
    this.currentIndex = 0
    this.loadedTabs = []
    this.tabs = []

    Channel.on('core:tab:open', (_, template) => {
      let tab = Tab.getTemplate(template)
      if (tab.singleton === true) {
        let index = this.tabs.findIndex((t) => { return t.tab.template === template })
        if (index >= 0) {
          this.currentIndex = index
          m.redraw()
          return
        }
      }
      this.tabs.push({
        id: this.idCounter++,
        tab: Object.assign({}, tab)
      })
      this.currentIndex = this.tabs.length - 1
      m.redraw()
    })

    Channel.on('core:tab:set-name', (_, id, name) => {
      let item = this.tabs.find((t) => { return t.id === id })
      if (item) {
        item.tab.name = name
        m.redraw()
      }
    })

    Channel.on('core:tab:focus', (_, id) => {
      let index = this.tabs.findIndex((t) => { return t.id === id })
      if (index >= 0) {
        this.currentIndex = index
        m.redraw()
      }
    })

    Channel.on('core:tab:close', (_, id) => {
      let index = this.tabs.findIndex((t) => { return t.id === id })
      if (index >= 0) {
        if (this.currentIndex > 0 && this.currentIndex === this.tabs.length - 1) {
          this.currentIndex--
        }
        this.tabs.splice(index, 1)
        m.redraw()
      }
    })

    ipcRenderer.on('tab-deplug-loaded', (_, id) => {
      this.loadedTabs[id] = true
      m.redraw()
    })

    Menu.register('core:tab:context', this.tabMenu)
  }

  activate(index) {
    this.currentIndex = parseInt(index)
  }

  tabMenu(menu, e) {
    menu.append(new MenuItem({
      label: 'Show Developer Tools',
      click: () => {
        let content = jquery(`webview:eq(${this.currentIndex})`).get()[0]
        content.openDevTools()
      }
    }))
    return menu
  }

  view() {
    return (
      <main>
        <div id="tab-container">
          {
            this.tabs.map((t, i) => {
              return (
                <a class="tab-label"
                  index={i}
                  isActive={ this.currentIndex === i }
                  onclick={m.withAttr('index', this.activate, this)}
                  oncontextmenu={ (e)=> {
                    Menu.popup('core:tab:context', this, remote.getCurrentWindow(), {event: e})
                  } }
                >
                <i class="fa fa-times"
                  onclick={() => {
                    if (this.currentIndex === i) {
                      Channel.emit('core:tab:close', t.id)
                    }
                  }}
                ></i>
                  { i18n(t.tab.name) }
                </a>
              )
            })
          }
        </div>
        <div id="menu-container">
          <a class="tab-menu-button"
            onclick={() => {Channel.emit('core:tab:open', 'Preferences')}}
          >
            <i class="fa fa-cog"></i>
          </a>
          <a class="tab-menu-button"
            onclick={() => {Channel.emit('core:tab:open', 'Pcap')}}
          >
            <i class="fa fa-plus"></i>
          </a>
        </div>
        <div id="tab-mask"></div>
        {
          this.tabs.map((t, i) => {
            return m(WebView, {
              key: t.id,
              id: t.id,
              index: i,
              item: t,
              isLoaded: (this.loadedTabs[t.id] === true),
              isActive: this.currentIndex === i
            })
          })
        }
      </main>
    )
  }
}
