import { Profile } from 'deplug'
import m from 'mithril'

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
    if ('min' in option && value < option.min) {
      value = option.min
    } else if ('max' in option && value > option.max) {
      value = option.max
    }
    this.updateValue(Math.floor(value), vnode)
  }
  updateStringValue (event, vnode) {
    const { option } = vnode.attrs
    let { value } = event.target
    if ('pattern' in option && !(new RegExp(option.pattern)).test(value)) {
      value = option.default || ''
    }
    if ('toJSON' in option) {
      value = option.toJSON(value)
    }
    this.updateValue(value, vnode)
  }
  updateEnumValue (event, vnode) {
    const { option } = vnode.attrs
    const value = event.target.options[event.target.selectedIndex].value ||
      option.default ||
      ''
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
    const defaultValue = ('default' in option)
      ? `Default: ${option.default}`
      : ''
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
            ('enumTitles' in option) ? option.enumTitles[index] : item
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
          placeholder: defaultValue,
        })
      case 'string':
        return m('input', {
          type: 'text',
          value,
          onchange: (event) => {
            this.updateStringValue(event, vnode)
          },
          placeholder: defaultValue,
        })
      default:
        return m('span', ['Unsupported Schema'])
    }
  }
}
