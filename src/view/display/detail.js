import { BufferValueItem, AttributeValueItem } from './value'
import AttributeItem from './attr'
import { ByteSlice } from '@genet/load-module'
import DefaultSummary from './default-summary'
import m from 'mithril'
import moment from 'moment'

let selectedLayer = null
function selectRange (range = null) {
  genet.action.emit('core:frame:range-selected', range)
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
    const addr = Number.parseInt(ByteSlice.address(layer.data), 10)
    const name = genet.session.tokenName(layer.id)

    const attrArray = [{
      key: layer.id,
      children: [],
    }]
    let prevDepth = 0
    const attrs = layer.attrs.map((attr, index) => ({
      attr,
      index,
    }))
    attrs.sort((lhs, rhs) => {
      if (lhs.attr.range[0] !== rhs.attr.range[0]) {
        return lhs.attr.range[0] - rhs.attr.range[0]
      }
      const llen = lhs.attr.range[1] - lhs.attr.range[0]
      const rlen = rhs.attr.range[1] - rhs.attr.range[0]
      if (llen !== rlen) {
        return rlen - llen
      }
      return lhs.index - rhs.index
    })
    for (const attr of attrs.map((item) => item.attr)) {
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
    const renderer = genet.session.layerRenderer(layer.id) || DefaultSummary
    return m('ul', [
      m('li', [
        m('details', { open: true }, [
          m('summary', {
            class: 'layer children',
            'data-layer': layer.id,
            active: selectedLayer.id === layer.id,
            onclick: () => {
              if (selectedLayer.id !== layer.id) {
                selectedLayer = layer
                genet.action.emit('core:frame:layer:selected', selectedLayer)
                return false
              }
            },
            onmouseover: () => selectRange({
              base: addr,
              length: layer.data.length,
            }),
            onmouseout: () => selectRange(),
            oncontextmenu: (event) => {
              genet.menu.showContextMenu(event, [
                {
                  label: `Apply Filter: ${layer.id}`,
                  click: () => genet.action
                    .emit('core:filter:set', layer.id),
                },
                {
                  label: 'Reveal in Developer Tools...',
                  click: () => {
                    // eslint-disable-next-line no-console
                    console.log(layer)
                    genet.action.global.emit('core:tab:open-devtool')
                  },
                }
              ])
            },
          }, [
            m('i', { class: 'fa fa-arrow-circle-right' }, [' ']),
            m('i', { class: 'fa fa-arrow-circle-down' }, [' ']),
            m('span', {
              class: 'protocol',
              'data-layer': layer.id,
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
              ' Stream #', layer.streamId])
          ]),
          mergeOrphanedItems(attrArray[0]).children
            .filter((item) => item.attr)
            .map((item) =>
              m(AttributeItem, {
                item,
                layer,
              })),
          m('ul', { class: 'metadata' }, [
            layer.payloads.map(
              (payload) => m('li', {
                onmouseover: () => selectRange({
                  base: Number.parseInt(ByteSlice.address(payload.data), 10),
                  length: payload.data.length,
                }),
                onmouseout: () => selectRange(),
              }, [
                m('detail', [
                  m('summary', [
                    m('span', [
                      ' ',
                      m(BufferValueItem, { value: Buffer.from(payload.data) }),
                      ' : ', payload.id, ' ', payload.type, ' '
                    ])
                  ])
                ])
              ]))
          ])
        ])
      ]),
      m('li', [
        m('ul', [
          layer.children.map((child) => m(LayerItem, { layer: child }))
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
    genet.action.on('core:frame:selected', (frame) => {
      this.selectedFrame = frame
      selectedLayer = frame
        ? frame.root
        : null
      genet.action.emit('core:frame:layer:selected', selectedLayer)
      m.redraw()
    })
    genet.action.on('core:filter:updated', (filter) => {
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
      moment(frame.query('link.timestamp').value * 1000)
        .format('YYYY-MM-DDTHH:mm:ss.SSSZ')

    const payload = frame.root.data
    const actual = frame.query('link.length').value
    let length = `${payload.length}`
    if (actual > payload.length) {
      length += ` (actual: ${actual})`
    }

    const children = frame.root.id.startsWith('[')
      ? frame.root.children
      : [frame.root]

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
