import m from 'mithril'
export default class Plugin {
  view () {
    return [
      m('h1', ['Package']),
      m('section', deplug.packages.list.map((pkg) => m('h4', [
          pkg.data.name, m('span', { class: 'schema-path' }, [pkg.data.version])
        ])))
    ]
  }

  oncreate () {
    deplug.packages.on('updated', () => {
      m.redraw()
    })
  }
}
