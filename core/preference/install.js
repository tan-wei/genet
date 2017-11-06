import m from 'mithril'
export default class Plugin {
  view () {
    return [
      m('h1', ['Install']),
      m('article', deplug.registry.packages.map((pkg) => m('h4', [pkg.name])))
    ]
  }

  oncreate () {
    deplug.registry.update()
    deplug.registry.on('updated', () => {
      m.redraw()
    })
  }
}
