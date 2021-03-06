import { AttributeValueItem } from './value'
import { ByteSlice } from '@genet/load-module'
import genet from '@genet/api'
import m from 'mithril'

function selectRange(range: any = null) {
  genet.action.emit('core:frame:range-selected', range)
}

export default class AttributeItem {
  view(vnode) {
    const { item, layer } = vnode.attrs
    const { attr, children } = item
    const addr = Number.parseInt(ByteSlice.address(layer.data), 10)
    let faClass = 'attribute'
    if (children.length) {
      faClass = 'attribute children'
    }
    const name = genet.session.tokenName(attr.id)
    const attrRenderer =
      genet.session.attrRenderer(attr.type) || AttributeValueItem
    return m('li', [
      m('details', [
        m('summary', {
          class: faClass,
          onmouseover: () => selectRange({
            base: addr + attr.range[0],
            length: attr.range[1] - attr.range[0],
          }),
          onmouseout: () => selectRange(),
          oncontextmenu: (event) => {
            genet.menu.showContextMenu(event, [
              {
                label: `Apply Filter: ${attr.filterExpression}`,
                click: () => genet.action
                  .emit('core:filter:set', attr.filterExpression),
              },
              {
                label: 'Reveal in Developer Tools...',
                click: () => {
                  // eslint-disable-next-line no-console
                  console.log(attr)
                  genet.action.global.emit('core:tab:open-devtool')
                },
              }
            ])
          },
        }, [
            m('span', { class: 'label' }, [
              m('i', { class: 'fa fa-circle-o' }, [' ']),
              m('i', { class: 'fa fa-arrow-circle-right' }, [' ']),
              m('i', { class: 'fa fa-arrow-circle-down' }, [' ']),
              name
            ]),
            m(attrRenderer, {
              attr,
              layer: vnode.attrs.layer,
            })
          ]),
        m('ul', [
          children.map((child) => m(AttributeItem, {
            item: child,
            layer,
          }))
        ])
      ])
    ])
  }
}
