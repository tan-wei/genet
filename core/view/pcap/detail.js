import { BufferValueItem, AttributeValueItem } from './value'
import AttributeItem from './attr'
import DefaultSummary from './default-summary'
import m from 'mithril'
import moment from '@deplug/moment.min'

let selectedLayer = null
function selectRange (range = []) {
  deplug.action.emit('core:frame:range-selected', range)
}

function mergeOrphanedItems (item) {
  const newChildren = []
  for (const child of item.children) {
    if (child.attr) {
      newChildren.push(child)
    } else {
      for (const grand of mergeOrphanedItems(child).children) {
        newChildren.push(grand)
      }
    }
  }
  return Object.assign(item, { children: newChildren })
}

class LayerItem {
  view (vnode) {
    const { layer } = vnode.attrs
    let dataOffset = 0
    if (layer.parent) {
      const [parentPayload] = layer.parent.payloads
      const parentAddr = parentPayload.slices[0].addr
      let rootPayload = parentPayload
      for (let { parent } = layer.parent; parent; { parent } = parent) {
        [rootPayload] = parent.payloads
      }
      const rootAddr = rootPayload.slices[0].addr
      dataOffset = parentAddr[1] - rootAddr[1]
    }
    const name = deplug.session.tokenName(layer.id)

    const attrArray = [{
      key: layer.id,
      children: [],
    }]
    let prevDepth = 0
    for (const attr of layer.attrs) {
      let { id } = attr
      if (id.startsWith('.')) {
        id = layer.id + id
      }
      const attrPath = id.split('.')
      let items = attrArray
      let child = null
      for (let index = 0; index < attrPath.length; index += 1) {
        const key = attrPath[index]
        if (index < attrPath.length - 1 || prevDepth < attrPath.length) {
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
      child.attr = attr
      prevDepth = attrPath.length
    }
    const renderer = deplug.session.layerRenderer(layer.id) || DefaultSummary
    return m('ul', [
      m('li', [
        m('details', { open: true }, [
          m('summary', {
            class: 'layer children',
            'data-layer': layer.id,
            'data-tags': layer.tags.join(' '),
            active: selectedLayer.id === layer.id,
            onclick: () => {
              if (selectedLayer.id !== layer.id) {
                selectedLayer = layer
                deplug.action.emit('core:frame:layer:selected', selectedLayer)
                return false
              }
            },
            onmouseover: () => selectRange(layer.range),
            onmouseout: () => selectRange(),
            oncontextmenu: (event) => {
              deplug.menu.showContextMenu(event, [
                {
                  label: `Apply Filter: ${layer.id}`,
                  click: () => deplug.action
                    .emit('core:filter:set', layer.id),
                },
                {
                  label: 'Reveal in Developer Tools...',
                  click: () => {
                    // eslint-disable-next-line no-console
                    console.log(layer)
                    deplug.action.global.emit('core:tab:open-devtool')
                  },
                }
              ])
            },
          }, [
            m('i', { class: 'fa fa-arrow-circle-right' }, [' ']),
            m('i', { class: 'fa fa-arrow-circle-down' }, [' ']),
            m('span', {
              class: 'protocol',
              'data-layer': layer.tags.join(' '),
            }, [
              name
            ]),
            m(renderer, { layer }),
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
          mergeOrphanedItems(attrArray[0]).children
            .filter((item) => item.attr)
            .map((item) =>
              m(AttributeItem, {
                item,
                layer,
                dataOffset,
              })),
          m('ul', { class: 'metadata' }, [
            layer.payloads.map(
              (payload) => payload.slices.map(
                (slice) => m('li', {
                  onmouseover: () => selectRange(payload.range),
                  onmouseout: () => selectRange(),
                }, [
                  m('detail', [
                    m('summary', [
                      m('span', [
                        ' ',
                        m(BufferValueItem, { value: slice }),
                        ' : ', payload.type, ' '
                      ])
                    ])
                  ])
                ])))
          ]),
          m('ul', { class: 'metadata error' }, [...layer.errors].map((err) =>
            m('li', [
              m('detail', [
                m('summary', [
                  m('span', [
                    m('i', { class: 'fa fa-exclamation-triangle' }), ' ',
                    deplug.session.tokenName(err.id), ' : ',
                    deplug.session.tokenName(err.target)
                  ])
                ])
              ])
            ])))
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
  }

  oncreate () {
    deplug.action.on('core:frame:selected', (frame) => {
      this.selectedFrame = frame
      selectedLayer = frame
        ? frame.primaryLayer
        : null
      deplug.action.emit('core:frame:layer:selected', selectedLayer)
      m.redraw()
    })
    deplug.action.on('core:filter:updated', (filter) => {
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
      moment(frame.query('$.timestamp').value)
        .format('YYYY-MM-DDTHH:mm:ss.SSSZ')

    const payload = frame.query('$.payload').value
    const actual = frame.query('$.actualLength').value
    let length = `${payload.length}`
    if (actual > payload.length) {
      length += ` (actual: ${actual})`
    }

    let children = frame.rootLayer.layers
    const rootId = frame.rootLayer.id
    if (rootId.startsWith('[')) {
      children = frame.rootLayer.layers
    }

    let filterValue = '(null)'
    if (this.displayFilter) {
      const value = this.displayFilter.built(frame)
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
          m('span', { class: 'label' }, [' Filter Result: ']),
          m('span', [
            this.displayFilter
              ? `${this.displayFilter.filter} =>`
              : '',
            ' ']),
          m('span', [m(AttributeValueItem, { attr: { value: filterValue } })])
        ])
      ]),
      children.map((layer) => m(LayerItem, { layer }))
    ])
  }
}
