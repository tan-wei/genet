import SchemaInput from './schema-input'
import m from 'mithril'
import { shell } from 'electron'
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
          m('span', { class: 'button-box' }, [
            m('input', {
              type: 'button',
              value: pkg.disabled
                ? 'Enable'
                : 'Disable',
              onclick: () => {
                if (pkg.disabled) {
                  deplug.packages.enable(pkg.data.name)
                } else {
                  deplug.packages.disable(pkg.data.name)
                }
              },
            }),
            m('input', {
              type: 'button',
              value: 'Open Website',
              style: {
                display: pkg.data.homepage
                  ? 'block'
                  : 'hidden',
              },
              onclick: () => {
                shell.openExternal(pkg.data.homepage)
              },
            }),
            m('input', {
              type: 'button',
              value: 'Open Directory',
              style: {
                display: pkg.builtin
                  ? 'none'
                  : 'block',
              },
              onclick: () => {
                shell.showItemInFolder(pkg.dir)
              },
            }),
            m('input', {
              type: 'button',
              value: 'Uninstall',
              style: {
                display: pkg.builtin
                  ? 'none'
                  : 'block',
              },
            })
          ]),
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
