import m from 'mithril'

class StringArrayInput {
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
}

class StringInput {
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
}

class IntegerInput {
  view (vnode) {
    const { schema } = vnode.attrs
    const placeholder = ('default' in schema)
      ? `Default: ${schema.default}`
      : ''
    return m('input', {
      type: 'number',
      value: deplug.config.get(vnode.attrs.id),
      placeholder,
    })
  }
}

class EnumInput {
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
}

export default class SchemaInput {
  view (vnode) {
    const { schema } = vnode.attrs
    if ('enum' in schema) {
      return m(EnumInput, vnode.attrs)
    }
    if (schema.type === 'array') {
      const { items } = schema
      if (items && items.type === 'string') {
        return m(StringArrayInput, vnode.attrs)
      }
    }
    switch (schema.type) {
      case 'string':
        return m(StringInput, vnode.attrs)
      case 'integer':
        return m(IntegerInput, vnode.attrs)
      default:
        return m('p', ['n/a'])
    }
  }
}
