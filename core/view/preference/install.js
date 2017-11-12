import m from 'mithril'
export default class Plugin {
  view () {
    return [
      m('h1', ['Install']),
      m('section', { class: 'package' }, deplug.registry.packages.map((pkg) => [
        m('h4', [
          pkg.name,
          m('span', { class: 'schema-path' },
          [pkg.version])
        ]),
        m('p', [pkg.description])
      ]
      ))
    ]
  }

  oncreate () {
    deplug.registry.on('updated', () => {
      m.redraw()
    })
    deplug.registry.update()
  }
}
