import m from 'mithril'

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
                src="https://www.github.com/"
              >
              </webview>
            )
          })
        }
      </main>
    )
  }
}
