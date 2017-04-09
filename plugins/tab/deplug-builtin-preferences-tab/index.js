import General from './general'
import Install from './install'
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
      case '/install':
        comp = Install
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
        <a
          href="#!/install"
          isactive={ page==='/install' }
        >Install</a>
      </nav>
      ,
      <main>
        { m(comp, vnode) }
      </main>
    ]
  }
}
