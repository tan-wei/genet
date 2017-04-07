import { ipcRenderer } from 'electron'
import { Argv, Channel, Tab } from 'deplug'
import m from 'mithril'
import jquery from 'jquery'

const tabs = []
const loadedTabs = []
let currentIndex = 0
let idCounter = 0

Channel.on('core:tab:open', (_, template) => {
  let tab = Tab.getTemplate(template)
  if (tab.singleton === true) {
    let index = tabs.findIndex((t) => { return t.tab.template === template })
    if (index >= 0) {
      currentIndex = index
      m.redraw()
      return
    }
  }
  tabs.push({
    id: idCounter++,
    tab: Object.assign({}, tab)
  })
  currentIndex = tabs.length - 1
  m.redraw()
})

Channel.on('core:tab:set-name', (_, id, name) => {
  let item = tabs.find((t) => { return t.id === id })
  if (item) {
    item.tab.name = name
    m.redraw()
  }
})

Channel.on('core:tab:focus', (_, id) => {
  let index = tabs.findIndex((t) => { return t.id === id })
  if (index >= 0) {
    currentIndex = index
    m.redraw()
  }
})

Channel.on('core:tab:close', (_, id) => {
  let index = tabs.findIndex((t) => { return t.id === id })
  if (index >= 0) {
    if (currentIndex > 0 && currentIndex === index && currentIndex === tabs.length - 1) {
      currentIndex--
    }
    tabs.splice(index, 1)
    m.redraw()
  }
})

ipcRenderer.on('tab-deplug-loaded', (_, id) => {
  loadedTabs[id] = true
  m.redraw()
})

class WebView {
  oncreate(vnode) {
    let webview = vnode.dom
    let item = tabs.find((t) => { return t.id === parseInt(vnode.attrs.id) })
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

export default class Main {
  activate(index) {
    currentIndex = parseInt(index)
    let content = jquery(`webview[index=${currentIndex}]`).get()[0]
  }

  view() {
    return (
      <main>
        <div id="tab-container">
          {
            tabs.map((t, i) => {
              return (
                <a class="tab-label"
                  index={i}
                  isActive={ currentIndex === i }
                  onclick={m.withAttr('index', this.activate, this)}
                >
                <i class="fa fa-times-circle"
                  onclick={() => {
                    if (currentIndex === i) {
                      Channel.emit('core:tab:close', t.id)
                    }
                  }}
                  style={{visibility: (currentIndex === i) ? 'visible' : 'hidden'}}
                ></i>
                  { t.tab.name }
                </a>
              )
            })
          }
          <a class="tab-menu-button"
            onclick={() => {Channel.emit('core:tab:open', 'Preferences')}}
          >
            <i class="fa fa-cog" aria-hidden="true"></i>
          </a>
          <a class="tab-menu-button"
            onclick={() => {Channel.emit('core:tab:open', 'Pcap')}}
          >
            <i class="fa fa-plus" aria-hidden="true"></i>
          </a>
        </div>
        <div id="tab-mask"></div>
        {
          tabs.map((t, i) => {
            return m(WebView, {
              key: t.id,
              id: t.id,
              index: i,
              isLoaded: (loadedTabs[t.id] === true),
              isActive: currentIndex === i
            })
          })
        }
      </main>
    )
  }
}
