import m from 'mithril'

export default class PackageView {
  constructor () {
    this.mode = 'local'
  }

  oncreate () {
    deplug.packages.on('updated', () => {
      m.redraw()
    })
    deplug.registry.on('updated', () => {
      m.redraw()
    })
    deplug.registry.update()
  }

  view () {
    return [
      m('nav', [
        m('div', { class: 'mode-selector' }, [
          m('button', {
            active: this.mode === 'local',
            onclick: () => {
              this.mode = 'local'
            },
          }, ['Local']),
          m('button', {
            active: this.mode === 'registry',
            onclick: () => {
              this.mode = 'registry'
            },
          }, ['Registry'])
        ]),
        m('div', {
          class: 'local-packages',
          style: {
            display: this.mode === 'local'
              ? 'block'
              : 'none',
          },
        }, [
          m('ul', deplug.packages.list.map((pkg) => m('li', [
            m('a', [
              m('h4', [pkg.data.name]),
              m('span', [pkg.data.description])
            ])])))
        ]),
        m('div', {
          class: 'registry-packages',
          style: {
            display: this.mode === 'registry'
              ? 'block'
              : 'none',
          },
        }, [
          m('ul', deplug.registry.packages.map((pkg) => m('li', [
            m('a', [
              m('h4', [pkg.data.name]),
              m('span', [pkg.data.description])
            ])])))
        ])
      ]),
      m('main'),
      m('div', { class: 'notification' })
    ]
  }
}
