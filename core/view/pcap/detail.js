import { BufferValueItem, AttributeValueItem } from './value'
import m from 'mithril'
import moment from '@deplug/moment.min'

function selectRange (range = []) {
  deplug.action.emit('core:frame:range-selected', range)
}

class AttributeItem {
  view (vnode) {
    const { property } = vnode.attrs
    const prop = property.attr
    const { children } = property
    let faClass = 'property'
    if (children.length) {
      faClass = 'property children'
    }
    const range = [
      prop.range[0] + vnode.attrs.dataOffset,
      prop.range[1] + vnode.attrs.dataOffset
    ]
    const { name } = deplug.session.token(prop.id)
    const propRenderer =
      deplug.session.attrRenderer(prop.type) || AttributeValueItem
    return m('li', {
      'data-range': `${range[0]}:${range[1]}`,
      onmouseover: () => selectRange(range),
      onmouseout: () => selectRange(),
    }, [
      m('details', [
        m('summary', { class: faClass }, [
          m('span', { class: 'label' }, [
            m('i', { class: 'fa fa-circle-o' }, [' ']),
            m('i', { class: 'fa fa-arrow-circle-right' }, [' ']),
            m('i', { class: 'fa fa-arrow-circle-down' }, [' ']),
            name, ': '
          ]),
          m(propRenderer, {
            prop,
            layer: vnode.attrs.layer,
          }),
          m('span', {
            class: 'label error',
            style: {
              display: prop.error
              ? 'inline'
              : 'none',
            },
          }, [m('i', { class: 'fa fa-exclamation-triangle' }), ' ', prop.error])
        ]),
        m('ul', [
          children.map((child) => m(AttributeItem, {
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
    const { name } = deplug.session.token(layerId)

    const properties = layer.attrs
    const propArray = []
    let prevDepth = 0
    for (const prop of properties) {
      let { id } = prop
      if (id.startsWith('.')) {
        id = layer.id + id
      }
      const propPath = id.split('.')
      let items = propArray
      let child = null
      for (let index = 0; index < propPath.length; index += 1) {
        const key = propPath[index]
        if (index < propPath.length - 1 || prevDepth < propPath.length) {
          child = items.find((item) => item.key === key) || null
        } else {
          child = null
        }
        if (child === null) {
          child = {
            key,
            children: [],
          }
          items.push(child)
        }
        items = child.children
      }
      child.attr = prop
      prevDepth = propPath.length
    }
    return m('ul', [
      m('li', {
        'data-range': `${range[0]}:${range[1]}`,
        onmouseover: () => selectRange(range),
        onmouseout: () => selectRange(),
      }, [
        m('details', {
          open: true,
          oncontextmenu: (event) => {
            deplug.menu.showContextMenu(event, [
              { label: `Apply Filter: ${layerId}` }
            ])
          },
        }, [
          m('summary', {
            class: 'layer children',
            'data-layer': layer.tags.join(' '),
          }, [
            m('i', { class: 'fa fa-arrow-circle-right' }, [' ']),
            m('i', { class: 'fa fa-arrow-circle-down' }, [' ']),
            name, ' ',
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
          propArray[0].children.map((prop) =>
            m(AttributeItem, {
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

export default class PcapDetailView {
  constructor () {
    this.selectedFrame = null
    this.displayFilter = null
    deplug.action.on('core:frame:selected', (frames) => {
      this.selectedFrame = frames[0] || null
      m.redraw()
    })
  }

  oncreate () {
    deplug.action.on('core:filter:set', (filter) => {
      this.displayFilter = filter
      m.redraw()
    })
  }

  view () {
    if (this.selectedFrame === null) {
      return m('div', { class: 'detail-view' }, ['No frame selected'])
    }
    const frame = this.selectedFrame

    const tsString =
      moment(frame.timestamp).format('YYYY-MM-DDTHH:mm:ss.SSSZ')

    let length = `${frame.rootLayer.payloads[0].slices[0].length}`
    if (frame.length > frame.rootLayer.payloads[0].slices[0].length) {
      length += ` (actual: ${frame.length})`
    }

    let children = frame.rootLayer.layers
    const rootId = frame.rootLayer.id
    if (rootId.startsWith('[')) {
      children = frame.rootLayer.layers
    }

    let filterValue = '(null)'
    if (this.displayFilter) {
      const value = this.displayFilter.test(frame)
      if (value !== null) {
        filterValue = value
      }
    } else {
      filterValue = 'No filter'
    }

    return m('div', { class: 'detail-view' }, [
      m('ul', [
        m('li', [
          m('i', { class: 'fa fa-circle-o' }),
          m('span', { class: 'label' }, [' Timestamp: ']),
          m('span', [' ', tsString, ' '])
        ]),
        m('li', [
          m('i', { class: 'fa fa-circle-o' }),
          m('span', { class: 'label' }, [' Length: ']),
          m('span', [' ', length, ' '])
        ]),
        m('li', [
          m('i', { class: 'fa fa-circle-o' }),
          m('span', { class: 'label' }, [' Evaluated Filter: ']),
          m('span', [
            this.displayFilter
            ? `${this.displayFilter.expression} =>`
            : '',
           ' ']),
          m('span', [m(AttributeValueItem, { prop: { value: filterValue } })])
        ])
      ]),
      children.map((layer) => m(LayerItem, { layer }))
    ])
  }
}
