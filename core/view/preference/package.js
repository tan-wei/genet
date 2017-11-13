import SchemaInput from './schema-input'
import m from 'mithril'
export default class Plugin {
  view () {
    return [
      m('h1', ['Package']),
      m('div', deplug.packages.list.map((pkg) => {
        const config = pkg.disabled
          ? []
          : Object.entries(deplug.config.schema)
            .filter(([id]) => id.startsWith(`${pkg.data.name}.`))
        return m('section', {
          class: 'package',
          disabled: pkg.disabled,
          }, [
          m('h4', [
            pkg.data.name,
            m('span', { class: 'schema-path' },
            [pkg.data.version])
          ]),
          m('p', [pkg.data.description]),
          config.map(([id, schema]) => m('section', [
              m('h4', [
                schema.title, m('span', { class: 'schema-path' }, [id])]),
              m(SchemaInput, {
                id,
                schema,
              }),
              m('p', { class: 'description' }, [schema.description])
          ]))
        ])
      }
    ))
    ]
  }

  oncreate () {
    deplug.packages.on('updated', () => {
      m.redraw()
    })
  }
}
