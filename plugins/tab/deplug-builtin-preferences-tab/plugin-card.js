import OptionView from './option-view'
import { Profile } from 'deplug'
import m from 'mithril'

export default class PluginCard {
  disable(pkg) {
    Profile.current[`$${pkg.name}`].enabled = false
  }

  enable(pkg) {
    Profile.current[`$${pkg.name}`].enabled = true
  }

  view(vnode) {
    const pkg = vnode.attrs.pkg
    const options = vnode.attrs.options || []
    const enabled = Profile.current[`$${pkg.name}`].enabled
    const name = pkg.name
      .replace(/^(deplug-builtin-|deplugin-)/, '')
    const builtin = pkg.name.startsWith('deplug-builtin-')
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
