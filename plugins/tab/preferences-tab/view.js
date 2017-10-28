import About from './about'
import General from './general'
import Install from './install'
import Plugin from './plugin'
import Profile from './profile'
import { Tab } from 'deplug'
import m from 'mithril'

export default class View {
  view (vnode) {
    let comp = null
    switch (Tab.page) {
      case 'plugin':
        comp = Plugin
        break
      case 'install':
        comp = Install
        break
      case 'profile':
        comp = Profile
        break
      case 'about':
        comp = About
        break
      default:
        comp = General
    }

    return [
      m('nav', [
        m('a', {
          href: 'javascript:void(0)',
          onclick: () => {
            Tab.page = ''
          },
          isactive: Tab.page === '',
        }, ['General']),
        m('a', {
          href: 'javascript:void(0)',
          onclick: () => {
            Tab.page = 'plugin'
          },
          isactive: Tab.page === 'plugin',
        }, ['Plugin']),
        m('a', {
          href: 'javascript:void(0)',
          onclick: () => {
            Tab.page = 'install'
          },
          isactive: Tab.page === 'install',
        }, ['Install']),
        m('a', {
          href: 'javascript:void(0)',
          onclick: () => {
            Tab.page = 'profile'
          },
          isactive: Tab.page === 'profile',
        }, ['Profile']),
        m('a', {
          href: 'javascript:void(0)',
          onclick: () => {
            Tab.page = 'about'
          },
          isactive: Tab.page === 'about',
        }, ['About Deplug'])
      ]),
      m('main', [
        m(comp, vnode)
      ])
    ]
  }
}
