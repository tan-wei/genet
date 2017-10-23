import {
  Profile
} from 'deplug'
import m from 'mithril'
export default class ProfileView {
  view(vnode) {
    return [
      m('h1', ['Profile']),
      m('table', [
        m('tr', [
          m('td', ['Current Profile']),
          m('td', [Profile.currentId])
        ])
      ])
    ]
  }
}
