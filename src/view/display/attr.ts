import { AttributeValueItem } from './value'
import { Bytes } from '@genet/load-module'
import genet from '@genet/api'
import m from 'mithril'

function selectRange(range: any = null) {
  genet.action.emit('core:frame:range-selected', range)
}

export default class AttributeItem {
  view(vnode) {
    const { item, layer } = vnode.attrs
    const { attr, children } = item
    const addr = Bytes.address(layer.data)
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
            base: addr + BigInt(attr.range[0]),
            length: BigInt(attr.range[1] - attr.range[0]),
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
          children
            .filter((item) => item.attr && item.attr.value !== null)
            .map((item) => m(AttributeItem, {
              item,
              layer,
            }))
        ])
      ])
    ])
  }
}
