import General from './general'
import Install from './install'
import Plugin from './plugin'
import { Tab } from 'deplug'
import m from 'mithril'

export default class View {
  view(vnode) {
    let comp = General
    switch (Tab.page) {
      case 'plugin':
        comp = Plugin
        break
      case 'install':
        comp = Install
        break
    }
    return [
      <nav>
        <a
          href="javascript:void(0)"
          onclick={ () => { Tab.page = '' } }
          isactive={ Tab.page==='' }
        >General</a>
        <a
          href="javascript:void(0)"
          onclick={ () => { Tab.page = 'plugin' } }
          isactive={ Tab.page==='plugin' }
        >Plugin</a>
        <a
          href="javascript:void(0)"
          onclick={ () => { Tab.page = 'install' } }
          isactive={ Tab.page==='install' }
        >Install</a>
      </nav>
      ,
      <main>
        { m(comp, vnode) }
      </main>
    ]
  }
}
