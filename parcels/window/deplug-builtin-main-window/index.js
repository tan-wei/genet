import m from 'mithril'
import jquery from 'jquery'

const tabs = [
  {
    name: "Preferences"
  },
  {
    name: "Live capture 1"
  }
]

let currentIndex = 0

class WebView {
  constructor() {

  }

  view(vnode) {
    return <webview
      class="tab-content"
      src={`file://${__dirname}/index.htm`}
      isActive={vnode.attrs.isActive}
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
                  onclick={m.withAttr('index', this.activate)}
                >
                  { t.name }
                </a>
              )
            })
          }
        </div>
        {
          tabs.map((t, i) => {
            return m(WebView, {index: i, isActive: currentIndex === i })
          })
        }
      </main>
    )
  }
}
