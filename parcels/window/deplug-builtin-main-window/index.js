import m from 'mithril'

const tabs = [
  {
    name: "Preferences"
  },
  {
    name: "Live capture 1"
  }
]

export default class Main {
  view() {
    return (
      <main>
        <div id="tab-container">
          {
            tabs.map((t, i) => {
              return (
                <a class="tab-label" index={i} isActive={ 1 === i }>
                  { t.name }
                </a>
              )
            })
          }
        </div>
        <webview class="tab-content" src="https://www.github.com/"></webview>
      </main>
    )
  }
}
