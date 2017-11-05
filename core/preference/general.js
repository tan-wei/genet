import SchemaInput from './schema-input'
import m from 'mithril'
import schemaDefault from '../lib/schema-default'

export default class General {
  view () {
    const config = Object.entries(schemaDefault.config)
      .filter(([id]) => id.startsWith('_.'))
    return [
      m('h1', ['General']),
      config.map(([id, schema]) => m('section', [
          m('h4', [schema.title, m('span', { class: 'schema-path' }, [id])]),
          m(SchemaInput, {
            id,
            schema,
          }),
          m('p', { class: 'description' }, [schema.description])
      ]))
    ]
  }
}
