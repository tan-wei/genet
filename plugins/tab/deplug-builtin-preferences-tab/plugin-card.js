import { Profile } from 'deplug'
import m from 'mithril'

class OptionView {
  view(vnode) {
    const { pkg, option } = vnode.attrs
    const value = Profile.current[`$${pkg.name}`][option.id]
    switch (option.type) {
      case 'boolean':
        return <input type="checkbox" checked></input>
      case 'integer':
        return <input type="number" value={ value }></input>
      default:
        return <span>n/a</span>
    }
  }
}

export default class PluginCard {
  view(vnode) {
    const pkg = vnode.attrs.pkg
    const options = vnode.attrs.options || []
    let name = pkg.name
      .replace(/^(deplug-builtin-|deplugin-)/, '')
    return <div class="card">
      <a>{ name } <small>({ pkg.version })</small></a>
      <table style={{display: vnode.attrs.installed ? 'block' : 'none'}}>
        <tr>
          <td>Enabled</td>
          <td>{ m(OptionView, {pkg, option: {id: 'enabled', type: 'boolean'}}) }</td>
        </tr>
        {
          options.map((opt) => {
            return <tr>
              <td>{ opt.name }</td>
              <td>{ m(OptionView, {pkg, option: opt}) }</td>
            </tr>
          })
        }
      </table>
    </div>
  }
}
