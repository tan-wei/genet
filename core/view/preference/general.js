import SchemaInput from './schema-input'
import m from 'mithril'

export default class General {
  view (vnode) {
    const config = Object.entries(deplug.config.schema)
      .filter(([id]) => id.startsWith(vnode.attrs.prefix))
    return [
      m('h1', [vnode.attrs.name]),
      m('div',
        config.map(([id, schema]) => m('section', [
          m('h4', [schema.title, m('span', { class: 'schema-path' }, [id])]),
          m(SchemaInput, {
            id,
            schema,
          }),
          m('p', { class: 'description' }, [schema.description])
      ])))
    ]
  }

  oncreate () {
    deplug.config.watch('_', () => {
      m.redraw()
    })
  }
}
