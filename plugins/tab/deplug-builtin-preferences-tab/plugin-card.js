import { Profile } from 'deplug'
import m from 'mithril'

class OptionView {
  updateBooleanValue(event, vnode) {
    this.updateValue(event.target.checked, vnode)
  }

  updateIntegerValue(event, vnode) {
    const { option } = vnode.attrs
    let value = Number.parseInt(event.target.value)
    if (Number.isNaN(value)) {
      value = option.default || 0
    }
    if ('min' in option && value < option.min) {
      value = option.min
    } else if ('max' in option && value > option.max) {
      value = option.max
    }
    this.updateValue(Math.floor(value), vnode)
  }

  updateStringValue(event, vnode) {
    const { option } = vnode.attrs
    let value = event.target.value
    if ('regexp' in option && !(new RegExp(option.regexp)).test(value)) {
      value = option.default || 0
    }
    this.updateValue(value, vnode)
  }

  updateEnumValue(event, vnode) {
    const { option } = vnode.attrs
    let value = event.target.options
      [event.target.selectedIndex].value || option.default || ''
    this.updateValue(value, vnode)
  }

  updateValue(value, vnode) {
    const { pkg, option } = vnode.attrs
    if (value === option.default) {
      delete Profile.current[`$${pkg.name}`][option.id]
    } else {
      Profile.current[`$${pkg.name}`][option.id] = value
    }
  }

  view(vnode) {
    const { pkg, option } = vnode.attrs
    const value = Profile.current[`$${pkg.name}`][option.id]
    const defaultValue = ('default' in option) ? `Default: ${option.default}` : ''
    switch (option.type) {
      case 'boolean':
        return <input
          type="checkbox"
          onclick={ (event) => { this.updateBooleanValue(event, vnode) } }
          checked={value}
          ></input>
      case 'integer':
        return <input
          type="number"
          value={ value }
          onchange={ (event) => { this.updateIntegerValue(event, vnode) } }
          placeholder={ defaultValue }
          ></input>
      case 'string':
        return <input
          type="text"
          value={ value }
          onchange={ (event) => { this.updateStringValue(event, vnode) } }
          placeholder={ defaultValue }
          ></input>
      case 'enum':
        return <select
          onchange={ (event) => { this.updateEnumValue(event, vnode) } }
          >
          {
            (option.values).map((item) => {
              return <option
                selected={ item.value == value }
                value={ item.value }>
                { item.name }</option>
            })
          }
        </select>
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
      <div class="title">
        <span>{ name } <small>({ pkg.version })</small></span>
        <span>
          <input
            type="button"
            value="Disable"
          ></input>
        </span>
        <span>
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
