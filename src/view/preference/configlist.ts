import SchemaInput from '../../lib/schema-input'
import genet from '@genet/api'
import titleCase from 'title-case'
import m from 'mithril'
import marked from 'marked'

class Markdown {
  view() {
    return m('p', { class: 'description' })
  }

  oncreate(vnode) {
    vnode.dom.innerHTML = marked(vnode.attrs.content)
  }
}

export default class ConfigList {
  view(vnode) {
    const config = Object.entries(genet.config.schema)
      .filter(([id]) => id.startsWith(vnode.attrs.prefix)) as [string, any][]
    return [
      m('div',
        config.map(([id, schema]) => m('section', [
          m('h4', [
            schema.title || titleCase(id.split('.').slice(-1)[0]),
            m('span', { class: 'schema-path' }, [id])
          ]),
          m(SchemaInput, {
            id,
            schema,
          }),
          m(Markdown, { content: schema.description || '' })
        ])))
    ]
  }

  oncreate() {
    genet.config.watch('_', () => {
      m.redraw()
    })
  }
}
