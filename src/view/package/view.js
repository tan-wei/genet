import DetailView from './detail'
import m from 'mithril'

export default class PackageView {
  constructor () {
    this.mode = 'local'
    this.selectedLocalPackage = ''
    this.selectedRegistryPackage = ''
  }

  oncreate () {
    genet.packages.on('updated', () => {
      m.redraw()
    })
    genet.registry.on('updated', () => {
      m.redraw()
    })
    genet.registry.update()
  }

  view () {
    if (genet.packages.list.map((pkg) => pkg.id)
      .indexOf(this.selectedLocalPackage) < 0) {
      if (genet.packages.list.length > 0) {
        this.selectedLocalPackage = genet.packages.list[0].id
      } else {
        this.selectedLocalPackage = ''
      }
    }
    if (genet.registry.packages.map((pkg) => pkg.id)
      .indexOf(this.selectedRegistryPackage) < 0) {
      if (genet.registry.packages.length > 0) {
        this.selectedRegistryPackage = genet.registry.packages[0].id
      } else {
        this.selectedRegistryPackage = ''
      }
    }
    let selectedPackage = null
    if (this.mode === 'local') {
      selectedPackage = genet.packages.list.find((pkg) =>
        pkg.id === this.selectedLocalPackage) || null
    } else {
      selectedPackage = genet.registry.packages.find((pkg) =>
        pkg.id === this.selectedRegistryPackage) || null
    }
    if (selectedPackage !== null) {
      const installedPkg = genet.packages.get(selectedPackage.id)
      selectedPackage = installedPkg || selectedPackage
    }
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
          m('ul', genet.packages.list.map((pkg) =>
            m('li', [
              m('a', {
                active: this.selectedLocalPackage === pkg.id,
                onclick: () => {
                  this.selectedLocalPackage = pkg.id
                },
              }, [
                m('h4', { disabled: pkg.disabled || pkg.incompatible }, [
                  pkg.data.name
                ]),
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
          m('ul', genet.registry.packages.map((pkg) =>
            m('li', [
              m('a', {
                active: this.selectedRegistryPackage === pkg.id,
                onclick: () => {
                  this.selectedRegistryPackage = pkg.id
                },
              }, [
                m('h4', [pkg.data.name]),
                m('span', [pkg.data.description])
              ])])))
        ])
      ]),
      m('main', [m(DetailView, { pkg: selectedPackage })]),
      m('div', { class: 'notification' })
    ]
  }
}
