import { Profile, PluginRegistry } from 'deplug'
import PluginCard from './plugin-card'
import m from 'mithril'
import moment from 'moment'
let loading = false
const registry =
  new PluginRegistry(Profile.current.get('_.pluginRegistries'))
export default class InstallView {
  constructor () {
    this.plugins = registry.plugins
  }

  oncreate () {
    if (registry.lastUpdated === null ||
      moment(registry.lastUpdated)
        .isBefore(moment().subtract(1, 'hours'))) {
      this.fetchPlugins()
    }
  }

  fetchPlugins () {
    if (!loading) {
      loading = true
      registry.once('updated', () => {
        loading = false
        this.plugins = registry.plugins
        m.redraw()
      })
      registry.update()
      m.redraw()
    }
  }

  view () {
    const lastUpdated = moment(registry.lastUpdated).fromNow()
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
            this.fetchPlugins()
          },
        }, [
          m('i', { class: 'fa fa-check' }), ` Last updated: ${lastUpdated}`
        ])
      ]),
      m('div', [
        this.plugins.map((pkg) => m(PluginCard, { pkg }))
      ])
    ]
  }
}
