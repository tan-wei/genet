import m from 'mithril'
import jquery from 'jquery'
import { Argv } from 'deplug'

const tabs = [
  {
    name: "Preferences"
  },
  {
    name: "Live capture 1"
  }
]

class WebView {
  constructor() {

  }

  oncreate(vnode) {
    let webview = vnode.dom
    webview.addEventListener('dom-ready', () => {
      let argv = JSON.stringify(Argv)
      const script = `require("deplug-core/tab.main")(${argv})`
      webview.executeJavaScript(script)
    })
  }

  view(vnode) {
    return <webview
      class="tab-content"
      src={`file://index.htm`}
      isActive={vnode.attrs.isActive}
      nodeintegration
    >
    </webview>
  }
}

export default class Main {
  constructor() {
    this.currentIndex = 0
  }

  activate(index) {
    this.currentIndex = parseInt(index)
    let content = jquery(`webview[index=${this.currentIndex}]`).get()[0]
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
                  isActive={ this.currentIndex === i }
                  onclick={m.withAttr('index', this.activate, this)}
                >
                  { t.name }
                </a>
              )
            })
          }
        </div>
        {
          tabs.map((t, i) => {
            return m(WebView, {index: i, isActive: this.currentIndex === i })
          })
        }
      </main>
    )
  }
}
