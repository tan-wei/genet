import DetailView from './detail'
import genet from '@genet/api'
import m from 'mithril'

export default class PackageView {
  constructor () {
    this.selectedLocalPackage = ''
  }

  oncreate () {
    genet.packages.on('updated', () => {
      m.redraw()
    })
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
    let selectedPackage = genet.packages.list.find((pkg) =>
      pkg.id === this.selectedLocalPackage) || null
    if (selectedPackage !== null) {
      const installedPkg = genet.packages.get(selectedPackage.id)
      selectedPackage = installedPkg || selectedPackage
    }
    return [
      m('nav', [
        m('div', { class: 'local-packages' }, [
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
        ])
      ]),
      m('main', [m(DetailView, { pkg: selectedPackage })]),
      m('div', { class: 'notification' })
    ]
  }
}
