import genet from '@genet/api'
import m from 'mithril'
export default class KeyBind {
  oncreate() {
    genet.keybind.on('update', () => {
      m.redraw()
    })
  }
  view() {
    const map = { ...genet.keybind.keymap }
    for (const [combo, binds] of Object.entries(genet.menu.keymap)) {
      map[combo] = (map[combo] || []).concat(binds)
    }
    const combinations: any[] = []
    for (const [combo, binds] of Object.entries(map)) {
      for (const bind of binds as any[]) {
        combinations.push({ combo, ...bind })
      }
    }
    return m('table', [
      m('tr', [
        m('th', ['Combo']),
        m('th', ['Selector']),
        m('th', ['Action']),
        m('th', ['Label'])
      ])
    ].concat(combinations.map(({ combo, selector, action, label }) => m('tr', [
      m('td', [combo]),
      m('td', [selector]),
      m('td', [action]),
      m('td', [label])
    ]))))
  }
}
