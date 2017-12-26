import m from 'mithril'
import moment from '@deplug/moment.min'

 class BooleanValueItem {
  view (vnode) {
    const faClass = vnode.attrs.value
      ? 'fa-check-square-o'
      : 'fa-square-o'
    return m('span', [m('i', { class: `fa ${faClass}` })])
  }
}
 class DateValueItem {
  view (vnode) {
    const ts = moment(vnode.attrs.value)
    const tsString = ts.format('YYYY-MM-DDTHH:mm:ss.SSSZ')
    return m('span', [tsString])
  }
}
export class BufferValueItem {
  view (vnode) {
    const maxLen = 6
    const buffer = vnode.attrs.value
    const hex = buffer.slice(0, maxLen).toString('hex') +
      (buffer.length > maxLen
        ? '...'
        : '')
    return m('span', ['[', buffer.length, ' bytes] 0x', hex])
  }
}
 class ArrayValueItem {
  view (vnode) {
    return m('ul', [vnode.attrs.value.map(
      (value) => m('li', [m(AttributeValueItem, { prop: { value } })]))])
  }
}
 class ObjectValueItem {
  view (vnode) {
    const obj = vnode.attrs.value
    return m('ul', [Object.keys(obj).map(
      (key) => m('li', [m('span', { class: 'label' }, [key]),
      m(AttributeValueItem, { prop: { value: obj[key] } })]))])
  }
}
 class LayerValueItem {
  view (vnode) {
    const layer = vnode.attrs.value
    if (layer.payloads.length) {
      return m('span', [' [', layer.id, '] Payloads '])
    }
    return m('span', [' [', layer.id, '] '])
  }
}
export class AttributeValueItem {
  view (vnode) {
    const { prop } = vnode.attrs
    if (prop.value === null) {
      return m('span')
    } else if (typeof prop.value === 'boolean') {
      return m(BooleanValueItem, { value: prop.value })
    } else if (prop.value instanceof Date) {
      return m(DateValueItem, { value: prop.value })
    } else if (prop.value instanceof Uint8Array) {
      return m(BufferValueItem, { value: prop.value })
    } else if (Array.isArray(prop.value)) {
      return m(ArrayValueItem, { value: prop.value })
    } else if (typeof prop.value === 'object' &&
      prop.value.constructor.name === 'Layer') {
      return m(LayerValueItem, { value: prop.value })
    } else if (typeof prop.value === 'object' &&
      prop.value !== null &&
      Reflect.getPrototypeOf(prop.value) === Object.prototype) {
      return m(ObjectValueItem, { value: prop.value })
    }
    const value = (prop.value === null
      ? ''
      : prop.value.toString())
    if (value.length > 1024) {
      return m('span', [
        m('details', [
          m('summary', [value.substr(0, 64), '... (', value.length, ')']),
          value
        ])
      ])
    }
    return m('span', [' ', value, ' '])
  }
}
