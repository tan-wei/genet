import SchemaInput from './schema-input'
import m from 'mithril'

export default class General {
  view () {
    const config = Object.entries(deplug.config.schema)
      .filter(([id]) => id.startsWith('_.'))
    const sections = {}
    for (const [id, schema] of config) {
      const section = schema.section || 'Others'
      sections[section] = sections[section] || []
      sections[section].push([id, schema])
    }
    return [
      m('h1', ['General']),
      m('section',
        Object.entries(sections).map(([name, items]) => m('section',
        [
          m('h2', [name])
        ].concat(
          items.map(([id, schema]) => m('section', [
          m('h4', [schema.title, m('span', { class: 'schema-path' }, [id])]),
          m(SchemaInput, {
            id,
            schema,
          }),
          m('p', { class: 'description' }, [schema.description])
          ]))
        ))))
    ]
  }

  oncreate () {
    deplug.config.watch('_', () => {
      m.redraw()
    })
  }
}
