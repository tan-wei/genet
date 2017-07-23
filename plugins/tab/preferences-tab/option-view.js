import { Profile } from 'deplug'
import m from 'mithril'

export default class OptionView {
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
      value = option.default || ''
    }
    if ('toJSON' in option) {
      value = option.toJSON(value)
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
    const id = pkg ? pkg.name : '_'
    if (value === option.default) {
      Profile.current.delete(id, option.id)
    } else {
      Profile.current.set(id, option.id, value)
    }
  }

  view(vnode) {
    const { pkg, option } = vnode.attrs
    const id = pkg ? pkg.name : '_'
    let value = Profile.current.get(id, option.id)
    const defaultValue = ('default' in option) ? `Default: ${option.default}` : ''
    if (option.hasOwnProperty('toString')) {
      value = option.toString(value)
    }
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
