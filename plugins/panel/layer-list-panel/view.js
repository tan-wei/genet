import { Channel, Profile, Session, Renderer } from 'deplug'
import m from 'mithril'
import moment from 'moment'
import objpath from 'object-path'

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
    const tsformat = Profile.current.get('layer-list-panel', 'tsformat')
    const ts = moment(vnode.attrs.value)
    const tsString = ts.format(tsformat)
    return m('span', [tsString])
  }
}
class BufferValueItem {
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
      (value) => m('li', [m(PropertyValueItem, { prop: { value } })]))])
  }
}
class ObjectValueItem {
  view (vnode) {
    const obj = vnode.attrs.value
    return m('ul', [Object.keys(obj).map(
      (key) => m('li', [m('label', [key]),
      m(PropertyValueItem, { prop: { value: obj[key] } })]))])
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
class PropertyValueItem {
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
Renderer.registerProperty('', PropertyValueItem)

function selectRange (range = []) {
  Channel.emit('core:frame:range-selected', range)
}
const propSymbol = Symbol('prop')
const orderSymbol = Symbol('order')
function orderedProperties (obj) {
  return Object.values(obj)
    .sort((lhs, rhs) => lhs[orderSymbol] - rhs[orderSymbol])
    .map((item) => item)
}
class PropertyItem {
  view (vnode) {
    const prop = vnode.attrs.property[propSymbol]
    const children = orderedProperties(vnode.attrs.property)
    let faClass = 'property'
    if (children.length) {
      faClass = 'property children'
    }
    const range = [
      prop.range[0] + vnode.attrs.dataOffset,
      prop.range[1] + vnode.attrs.dataOffset
    ]
    const name = (prop.id in Session.attributes)
      ? Session.attributes[prop.id].name
      : prop.id
    const propRenderer = Renderer.forProperty(prop.type)
    return m('li', {
      'data-range': `${range[0]}:${range[1]}`,
      onmouseover: () => selectRange(range),
      onmouseout: () => selectRange(),
    }, [
      m('details', [
        m('summary', { class: faClass }, [
          m('label', [
            m('i', { class: 'fa fa-circle-o' }, [' ']),
            m('i', { class: 'fa fa-arrow-circle-right' }, [' ']),
            m('i', { class: 'fa fa-arrow-circle-down' }, [' ']),
            name, ': '
          ]),
          m(propRenderer, {
            prop,
            layer: vnode.attrs.layer,
          }),
          m('label', {
            class: 'error',
            style: {
              display: prop.error
              ? 'inline'
              : 'none',
            },
          }, [m('i', { class: 'fa fa-exclamation-triangle' }), ' ', prop.error])
        ]),
        m('ul', [
          children.map((child) => m(PropertyItem, {
              property: child,
              layer: vnode.attrs.layer,
            }))
        ])
      ])
    ])
  }
}
class LayerItem {
  view (vnode) {
    const { layer } = vnode.attrs
    let dataOffset = 0
    let dataLength = 0
    if (layer.parent) {
      const [parentPayload] = layer.parent.payloads
      const parentAddr = parentPayload.slices[0].addr
      let rootPayload = parentPayload
      for (let { parent } = layer.parent; parent; { parent } = parent) {
        [rootPayload] = parent.payloads
      }
      const rootAddr = rootPayload.slices[0].addr
      dataLength = parentPayload.slices[0].length
      dataOffset = parentAddr[1] - rootAddr[1]
    }
    const range = [
      dataOffset,
      dataOffset + dataLength
    ]
    const layerId = layer.id
    const name = (layerId in Session.attributes)
      ? Session.attributes[layerId].name
      : layerId
    const propObject = { [layerId]: {} }
    const properties = layer.attrs
    for (let index = 0; index < properties.length; index += 1) {
      const prop = properties[index]
      let { id } = prop
      if (id.startsWith('.')) {
        id = layer.id + id
      }
      objpath.ensureExists(propObject, id, {})
      const item = objpath.get(propObject, id)
      item[propSymbol] = prop
      item[orderSymbol] = index
    }
    return m('ul', [
      m('li', {
        'data-range': `${range[0]}:${range[1]}`,
        onmouseover: () => selectRange(range),
        onmouseout: () => selectRange(),
      }, [
        m('details', [
          m('summary', {
            class: 'layer children',
            'data-layer': layer.tags.join(' '),
          }, [
            m('i', { class: 'fa fa-arrow-circle-right' }, [' ']),
            m('i', { class: 'fa fa-arrow-circle-down' }, [' ']),
            name, ' ', Renderer.query(layer, '.'),
            m('span', {
               style: {
                display: layer.streamId > 0
                  ? 'inline'
                  : 'none',
                },
              },
              [
                m('i', { class: 'fa fa-exchange' }),
                ' Stream #', layer.streamId]),
                m('span', {
                style: {
                display: layer.confidence < 1.0
                                  ? 'inline'
                                  : 'none',
                },
              }, [
                m('i', { class: 'fa fa-question-circle' }),
                ' ',
                layer.confidence * 100, '%'
              ])
          ]),
          orderedProperties(propObject[layerId]).map((prop) => m(PropertyItem, {
              property: prop,
              layer,
              dataOffset,
            })),
          m('a', ['Payloads']),
          m('ul', [
            layer.payloads.map(
              (payload) => payload.slices.map(
                (slice) => m('li', [
                  ' ',
                  m(BufferValueItem, { value: slice }),
                  ' : ', payload.type, ' '])))
          ])
        ])
      ]),
      m('li', [
        m('ul', [
          layer.layers.map((child) => m(LayerItem, { layer: child }))
        ])
      ])
    ])
  }
}
export default class LayerListView {
  constructor () {
    this.frames = []
    Channel.on('core:frame:selected', (frames) => {
      this.frames = frames
      m.redraw()
    })
  }
  view () {
    if (!this.frames.length) {
      return m('div', { class: 'layer-list-view' }, ['No frames selected'])
    }
    return this.frames.map((frame) => {
      const tsformat = Profile.current.get('layer-list-panel', 'tsformat')
      const ts = moment(frame.timestamp)
      const tsString = ts.format(tsformat)
      let length = `${frame.rootLayer.payloads[0].slices[0].length}`
      if (frame.length > frame.rootLayer.payloads[0].slices[0].length) {
        length += ` (actual: ${frame.length})`
      }
      let children = frame.rootLayer.layers
      const rootId = frame.rootLayer.id
      if (rootId.startsWith('[')) {
        children = frame.rootLayer.layers
      }
      return m('div', { class: 'layer-list-view' }, [
        m('ul', [
          m('li', [
            m('i', { class: 'fa fa-circle-o' }),
            m('label', [' Timestamp: ']),
            m('span', [' ', tsString, ' '])
          ]),
          m('li', [
            m('i', { class: 'fa fa-circle-o' }),
            m('label', [' Length: ']),
            m('span', [' ', length, ' '])
          ])
        ]),
        children.map((layer) => m(LayerItem, { layer }))
      ])
    })
  }
}
