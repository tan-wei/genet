import { Config } from 'deplug'
import { Deplugin } from 'deplugin'
import PluginCard from './plugin-card'
import m from 'mithril'
import moment from 'moment'
let loading = false
const deplugin = new Deplugin(Config.deplug.version)
export default class InstallView {
  constructor () {
    this.packages = []
  }
  oncreate () {
    deplugin.cache().then((packages) => {
      this.packages = packages
      m.redraw()
      if (moment(deplugin.lastUpdated)
      .isBefore(moment().subtract(1, 'hours'))) {
        this.update()
      }
    })
  }
  update () {
    if (!loading) {
      loading = true
      m.redraw()
      deplugin.search().then((packages) => {
        this.packages = packages
        loading = false
        m.redraw()
      })
      .catch(() => {
        loading = false
        m.redraw()
      })
    }
  }
  view () {
    const lastUpdated = moment(deplugin.lastUpdated).fromNow()
    return [
      m('h1', ['Install Plugins ']),
      m('p', {
        style: {
          display: loading
          ? 'block'
          : 'none',
        },
      }, [
        m('i', { class: 'fa fa-refresh fa-spin' }), ' Fetching package list...'
      ]),
      m('p', {
        style: {
          display: loading
          ? 'none'
          : 'block',
        },
      }, [
        m('a', {
          href: 'javascript:void(0)',
          onclick: () => {
            this.update()
          },
        }, [
          m('i', { class: 'fa fa-check' }), ` Last updated: ${lastUpdated}`
        ])
      ]),
      m('div', [
        this.packages.map((pkg) => m(PluginCard, { pkg }))
      ])
    ]
  }
}
