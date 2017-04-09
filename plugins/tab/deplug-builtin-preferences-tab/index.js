import General from './general'
import Plugin from './plugin'
import m from 'mithril'

export default class View {
  view(vnode) {
    let page = location.hash.replace(/^#!/, '')
    let comp = General
    switch (page) {
      case '/plugin':
        comp = Plugin
        break
    }
    return [
      <nav>
        <a
          href="#!"
          isactive={ page==='' }
        >General</a>
        <a
          href="#!/plugin"
          isactive={ page==='/plugin' }
        >Plugin</a>
      </nav>
      ,
      <main>
        { m(comp, vnode) }
      </main>
    ]
  }
}
