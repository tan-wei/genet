import DetailView from './detail'
import InstallView from './install'
import genet from '@genet/api'
import m from 'mithril'

interface Metadata {
  name: string
  description?: string
}

interface Package {
  id: string
  source: string
  metadata: Metadata
}

export default class PackageView {
  private selected: string
  private packages: Package[]
  constructor() {
    this.selected = ''
    this.packages = []
  }

  update() {
    genet.gpm.list().then((list) => {
      this.packages = list
      m.redraw()
    })
  }

  oncreate() {
    genet.action.global.on('core:tab:reload', () => {
      window.location.reload()
    })

    this.update()

    genet.gpm.on('finish', (title, msg) => {
      genet.notify.show(
        msg, {
          type: 'success',
          title,
        })
    })

    genet.gpm.on('error', (title, msg) => {
      genet.notify.show(
        msg, {
          type: 'error',
          title,
        })
    })

    genet.gpm.on('reload', () => {
      this.update()
    })

    genet.gpm.on('update', () => {
      m.redraw()
    })
  }

  view() {
    if (this.packages.map((pkg) => pkg.id)
      .indexOf(this.selected) < 0) {
      this.selected = ':install:'
    }
    let selectedPackage = this.packages.find(({ id }) => id === this.selected)
    return [
      m('nav', [
        m('div', { class: 'local-packages' }, [
          m('ul', [
            m('li', [
              m('a', {
                active: this.selected === ':install:',
                onclick: () => {
                  this.selected = ':install:'
                },
              }, [
                  m('h4', [
                    'Install Package'
                  ]),
                  m('span', [m('i', { class: 'fa fa-gift' })])
                ])
            ]),
            m('li', { class: 'separator' })
          ].concat(this.packages.map((pkg) =>
            m('li', [
              m('a', {
                active: this.selected === pkg.id,
                onclick: () => {
                  this.selected = pkg.id
                },
              }, [
                  m('h4', { disabled: false }, [
                    pkg.metadata.name
                  ]),
                  m('span', [pkg.metadata.description])
                ])]))))
        ])
      ]),
      m('main', [
        m('div', {
          style: {
            display: this.selected === ':install:'
              ? 'block'
              : 'none',
          },
        }, [
            m(InstallView, {})
          ]),
        this.selected === ':install:'
          ? m('div')
          : m(DetailView, { pkg: selectedPackage })
      ]),
      m('div', { class: 'notification' })
    ]
  }
}
