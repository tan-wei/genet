import OptionView from './option-view'
import { Profile } from 'deplug'
import m from 'mithril'

export default class PluginCard {
  disable(pkg) {
    Profile.current.set('_', `disabledPlugins.${pkg.name}`, true)
  }

  enable(pkg) {
    Profile.current.delete('_', `disabledPlugins.${pkg.name}`)
  }

  view(vnode) {
    const pkg = vnode.attrs.pkg
    const options = vnode.attrs.options || []
    const enabled = !Profile.current.get('_', `disabledPlugins.${pkg.name}`, false)
    const name = pkg.name.replace(/^deplugin-/, '')
    const builtin = !pkg.name.startsWith('deplugin-')
    return <div class="card" enabled={enabled}>
      <div class="title">
        <span>{ name } <small>({ pkg.version })</small></span>
        <span style={{display: enabled ? 'block' : 'none'}}>
          <input
            type="button"
            value="Disable"
            onclick={ () => this.disable(pkg) }
          ></input>
        </span>
        <span style={{display: !enabled ? 'block' : 'none'}}>
          <input
            type="button"
            value="Enable"
            onclick={ () => this.enable(pkg) }
          ></input>
        </span>
        <span style={{display: vnode.attrs.installed && !builtin ? 'block' : 'none'}}>
          <input
            type="button"
            value="Unistall"
          ></input>
        </span>
      </div>
      <table style={{display: vnode.attrs.installed && options.length ? 'block' : 'none'}}>
        {
          options.map((opt) => {
            return <tr>
              <td data-tooltip={ `.${opt.id}` }>{ opt.name }</td>
              <td>{ m(OptionView, {pkg, option: opt}) }</td>
            </tr>
          })
        }
      </table>
    </div>
  }
}
