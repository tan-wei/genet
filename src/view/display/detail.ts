import { BufferValueItem, AttributeValueItem } from './value'
import AttributeItem from './attr'
import { ByteSlice } from '@genet/load-module'
import DefaultSummary from './default-summary'
import genet from '@genet/api'
import m from 'mithril'
import moment from 'moment'

let selectedLayer: any = null
function selectRange(range: any = null) {
  genet.action.emit('core:frame:range-selected', range)
}

function mergeOrphanedItems(item) {
  const newChildren: any[] = []
  for (const child of item.children) {
    if (child.attr) {
      newChildren.push(child)
    } else {
      for (const grand of mergeOrphanedItems(child).children) {
        newChildren.push(grand)
      }
    }
  }
  return { ...item, children: newChildren }
}

class LayerItem {
  view(vnode) {
    const { layer, frame } = vnode.attrs
    const addr = ByteSlice.address(layer.data)
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
      if (lhs.attr.bitRange[0] !== rhs.attr.bitRange[0]) {
        return lhs.attr.bitRange[0] - rhs.attr.bitRange[0]
      }
      const llen = lhs.attr.bitRange[1] - lhs.attr.bitRange[0]
      const rlen = rhs.attr.bitRange[1] - rhs.attr.bitRange[0]
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
      let child: any = null
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
              length: BigInt(layer.data.length),
            }),
            onmouseout: () => selectRange(),
            oncontextmenu: (event) => {
              genet.menu.showContextMenu(event, [
                {
                  label: `Apply Filter: ${layer.filterExpression}`,
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
              m(renderer, { layer, frame }),
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
            .filter((item) => item.attr && item.attr.value !== null)
            .map((item) =>
              m(AttributeItem, {
                item,
                layer,
              })),
        ])
      ]),
      m('li', [
        m('ul', [
          layer.children.map((child) => m(LayerItem, { layer: child, frame }))
        ])
      ])
    ])
  }
}

export default class PcapDetailView {
  private selectedFrame: any
  private displayFilter: any
  constructor() {
    this.selectedFrame = null
    this.displayFilter = null
  }

  oncreate() {
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

  view() {
    if (this.selectedFrame === null) {
      return m('div', { class: 'detail-view' }, ['No frame selected'])
    }
    const frame = this.selectedFrame

    const tsString =
      moment(Number(frame.query('link.timestamp').value) * 1000)
        .format('YYYY-MM-DDTHH:mm:ss.SSSZ')

    const payload = frame.root.data
    const actual = frame.query('link.originalLength').value
    let length = `${payload.length}`
    if (actual > payload.length) {
      length += ` (actual: ${actual})`
    }

    let children = [frame.root]
    while (children.every(layer => layer.id.startsWith('['))) {
      children = [].concat(...children.map(layer => layer.children))
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
      ]),
      children.map((layer) => m(LayerItem, { layer, frame }))
    ])
  }
}
