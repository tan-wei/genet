import m from 'mithril'
import { validate } from 'jsonschema'

class InputBase {
  writeValue (vnode, value) {
    const { id, schema } = vnode.attrs
    const result = validate(value, schema)
    if (result.errors.length === 0) {
      deplug.config.set(id, value)
    } else {
      if ('default' in schema) {
        deplug.config.set(id, schema.default)
      }
      deplug.notify.show(
        `${result.errors[0].name}: ${result.errors[0].message}`, {
          type: 'error',
          title: 'Validation failed',
        })
    }
  }
}

class StringArrayInput extends InputBase {
  view (vnode) {
    const { schema } = vnode.attrs
    const placeholder = ('default' in schema)
      ? schema.default.join(', ')
      : ''
    return m('input', {
      type: 'text',
      value: deplug.config.get(vnode.attrs.id).join(', '),
      placeholder,
    })
  }

  oncreate (vnode) {
    vnode.dom.addEventListener('change', (event) => {
      const value = event.target.value.split(',').map((item) => item.trim())
      this.writeValue(vnode, value)
    })
  }
}

class IntegerArrayInput extends InputBase {
  view (vnode) {
    const { schema } = vnode.attrs
    const placeholder = ('default' in schema)
      ? schema.default.join(', ')
      : ''
    return m('input', {
      type: 'text',
      value: deplug.config.get(vnode.attrs.id).join(', '),
      placeholder,
    })
  }

  oncreate (vnode) {
    vnode.dom.addEventListener('change', (event) => {
      const value = event.target.value.split(',')
        .map((item) => Number.parseInt(item.trim(), 10))
      this.writeValue(vnode, value)
    })
  }
}

class BooleanInput extends InputBase {
  view (vnode) {
    return m('input', {
      type: 'checkbox',
      checked: deplug.config.get(vnode.attrs.id),
    })
  }

  oncreate (vnode) {
    vnode.dom.addEventListener('change', (event) => {
      this.writeValue(vnode, event.target.checked)
    })
  }
}

class StringInput extends InputBase {
  view (vnode) {
    const { schema } = vnode.attrs
    const placeholder = ('default' in schema)
      ? schema.default
      : ''
    return m('input', {
      type: 'text',
      value: deplug.config.get(vnode.attrs.id),
      placeholder,
    })
  }

  oncreate (vnode) {
    vnode.dom.addEventListener('change', (event) => {
      const { value } = event.target
      this.writeValue(vnode, value)
    })
  }
}

class IntegerInput extends InputBase {
  view (vnode) {
    const { id, schema } = vnode.attrs
    const placeholder = ('default' in schema)
      ? `Default: ${schema.default}`
      : ''
    return m('input', {
      type: 'number',
      value: deplug.config.get(id),
      placeholder,
    })
  }

  oncreate (vnode) {
    vnode.dom.addEventListener('change', (event) => {
      const value = Number.parseInt(event.target.value, 10)
      this.writeValue(vnode, value)
    })
  }
}

class IntegerEnumInput extends InputBase {
  view (vnode) {
    const { schema } = vnode.attrs
    const value = deplug.config.get(vnode.attrs.id)
    const titles = schema.enumTitles || schema.enum
    return m('select', {}, [
      (schema.enum).map((item, index) => m('option', {
        selected: item === value,
        value: item,
      }, [
        titles[index]
      ]))
    ])
  }

  oncreate (vnode) {
    vnode.dom.addEventListener('change', (event) => {
      const value = Number.parseInt(
        event.target.options[event.target.selectedIndex].value, 10)
      this.writeValue(vnode, value)
    })
  }
}

class StringEnumInput extends InputBase {
  view (vnode) {
    const { schema } = vnode.attrs
    const value = deplug.config.get(vnode.attrs.id)
    const titles = schema.enumTitles || schema.enum
    return m('select', {}, [
      (schema.enum).map((item, index) => m('option', {
        selected: item === value,
        value: item,
      }, [
        titles[index]
      ]))
    ])
  }

  oncreate (vnode) {
    vnode.dom.addEventListener('change', (event) => {
      const { value } = event.target.options[event.target.selectedIndex]
      this.writeValue(vnode, value)
    })
  }
}

export default class SchemaInput {
  view (vnode) {
    const { schema } = vnode.attrs
    if ('enum' in schema) {
      switch (schema.type) {
        case 'string':
          return m(StringEnumInput, vnode.attrs)
        case 'integer':
          return m(IntegerEnumInput, vnode.attrs)
        default:
      }
    }
    if (schema.type === 'array') {
      const { items } = schema
      if (items) {
        switch (items.type) {
          case 'string':
            return m(StringArrayInput, vnode.attrs)
          case 'integer':
            return m(IntegerArrayInput, vnode.attrs)
          default:
        }
      }
    }
    switch (schema.type) {
      case 'string':
        return m(StringInput, vnode.attrs)
      case 'integer':
        return m(IntegerInput, vnode.attrs)
      case 'boolean':
        return m(BooleanInput, vnode.attrs)
      default:
        return m('p', ['n/a'])
    }
  }
}
