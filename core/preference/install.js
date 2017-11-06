import m from 'mithril'
export default class Plugin {
  view () {
    return [
      m('h1', ['Install']),
      m('section', deplug.registry.packages.map((pkg) => m('h4', [
          pkg.name, m('span', { class: 'schema-path' }, [pkg.version])
        ])))
    ]
  }

  oncreate () {
    deplug.registry.on('updated', () => {
      m.redraw()
    })
    deplug.registry.update()
  }
}
