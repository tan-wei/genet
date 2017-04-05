import m from 'mithril'

const User = {
  list: [
    {
      firstName: "hello",
      lastName: "world"
    }
  ]
}

export default class Main {
  view() {
    return (
      <main>
        <div id="tab-container"></div>
        <webview class="tab-content" src="https://www.github.com/"></webview>
      </main>
    )
  }
}
