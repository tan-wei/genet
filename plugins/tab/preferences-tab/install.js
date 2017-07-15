import { Deplugin } from 'deplugin'
import { Plugin, Config } from 'deplug'
import PluginCard from './plugin-card'
import m from 'mithril'
import moment from 'moment'

let loading = false
const deplugin = new Deplugin(Config.deplug.version)

export default class InstallView {
  constructor() {
    this.packages = []
  }

  oncreate() {
    deplugin.cache().then((packages) => {
      this.packages = packages
      m.redraw()
      if (moment(deplugin.lastUpdated)
        .isBefore(moment().subtract(1, 'hours'))) {
        this.update()
      }
    })
  }

  update() {
    if (!loading) {
      loading = true
      m.redraw()
      deplugin.search().then((packages) => {
        this.packages = packages
        loading = false
        m.redraw()
      }).catch(() => {
        loading = false
        m.redraw()
      })
    }
  }

  view(vnode) {
    const lastUpdated = moment(deplugin.lastUpdated).fromNow()
    return [
      <h1>Install Plugins </h1>,
      <p style={{display: loading ? 'block' : 'none'}}>
      <i class="fa fa-refresh fa-spin"></i> Fetching package list...</p>,
      <p style={{display: !loading ? 'block' : 'none'}}>
      <a href="javascript:void(0)" onclick={()=>{ this.update() }}>
      <i class="fa fa-check"></i>{` Last updated: ${lastUpdated}`}</a>
      </p>,
      <div>
        {
          this.packages.map((pkg) => {
            return m(PluginCard, {pkg: pkg})
          })
        }
      </div>
    ]
  }
}
