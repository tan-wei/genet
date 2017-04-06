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

export default class Main {
  activate(index) {
    currentIndex = parseInt(index)
    let content = jquery(`webview[index=${currentIndex}]`).get()[0]
    content.loadURL('https://github.com/')
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
            return (
              <webview class="tab-content"
                index={i}
                isActive={ currentIndex === i }
                src="blank"
              >
              </webview>
            )
          })
        }
      </main>
    )
  }
}
