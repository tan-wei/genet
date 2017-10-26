import { Profile } from 'deplug'
import m from 'mithril'
import objpath from 'object-path'

export default class OptionView {
  updateBooleanValue (event, vnode) {
    this.updateValue(event.target.checked, vnode)
  }
  updateIntegerValue (event, vnode) {
    const { option } = vnode.attrs
    let value = Number.parseInt(event.target.value, 10)
    if (Number.isNaN(value)) {
      value = option.default || 0
    }
    if (value < objpath.get(option, 'min', NaN)) {
      value = option.min
    } else if (objpath.get(option, 'max', NaN) > option.max) {
      value = option.max
    }
    this.updateValue(Math.floor(value), vnode)
  }
  updateStringValue (event, vnode) {
    const { option } = vnode.attrs
    let { value } = event.target
    if (!(new RegExp(objpath.get(option, 'pattern', ''))).test(value)) {
      value = option.default || ''
    }
    this.updateValue(value, vnode)
  }
  updateArrayValue (event, vnode) {
    const { option } = vnode.attrs
    let { value } = event.target
    const type = objpath.get(option, 'items.type', '')
    value = value.split(',').map((item) => {
      switch (type) {
        case 'number':
        case 'integer':
          return Number.parseInt(JSON.parse(item), 10)
        default:
          return item.trim()
      }
    })
    this.updateValue(value, vnode)
  }
  updateValue (value, vnode) {
    const { pkg, option, id } = vnode.attrs
    const pkgId = pkg
      ? pkg.name
      : '_'
    if (value === option.default) {
      Profile.current.delete(pkgId, id)
    } else {
      Profile.current.set(pkgId, id, value)
    }
  }
  view (vnode) {
    const { pkg, option, id } = vnode.attrs
    const pkgId = pkg
      ? pkg.name
      : '_'
    const value = Profile.current.get(pkgId, id)
    const defaultValue = objpath.get(option, 'default', '')
    if ('enum' in option) {
      return m('select', {
        onchange: (event) => {
          this.updateEnumValue(event, vnode)
        },
      }, [
        (option.enum).map((item, index) => m('option', {
            selected: item === value,
            value: item,
          }, [
            objpath.get(option, 'enumTitles', option.enum)[index]
          ]))
      ])
    }
    switch (option.type) {
      case 'boolean':
        return m('input', {
          type: 'checkbox',
          onclick: (event) => {
            this.updateBooleanValue(event, vnode)
          },
          checked: value,
        })
      case 'integer':
        return m('input', {
          type: 'number',
          value,
          onchange: (event) => {
            this.updateIntegerValue(event, vnode)
          },
          placeholder: `Default: ${defaultValue}`,
        })
      case 'string':
        return m('input', {
          type: 'text',
          value,
          onchange: (event) => {
            this.updateStringValue(event, vnode)
          },
          placeholder: `Default: ${defaultValue}`,
        })
      case 'array':
        return m('input', {
          type: 'text',
          value: value.join(', '),
          onchange: (event) => {
            this.updateArrayValue(event, vnode)
          },
          placeholder: Array.isArray(defaultValue)
            ? `Default: ${defaultValue.join(', ')}`
            : '',
        })
      default:
        return m('span', ['Unsupported Schema'])
    }
  }
}
