import { AttributeValueItem } from './value'
import m from 'mithril'

function selectRange (range = []) {
  genet.action.emit('core:frame:range-selected', range)
}

function filterExpression (attr) {
  const macro = genet.session.attrMacro(attr.type)
  if (macro !== null) {
    const exp = macro(attr)
    const prefix = '@'
    return `${attr.id} == ${prefix}${exp}`
  }
  const value = attr.getValue()
  if (value === true) {
    return attr.id
  } else if (value === false) {
    return `!${attr.id}`
  } else if (value instanceof Uint8Array) {
    return `${attr.id} == ${JSON.stringify(Array.from(value))}`
  }
  return `${attr.id} == ${JSON.stringify(value)}`
}

export default class AttributeItem {
  view (vnode) {
    const { item, layer } = vnode.attrs
    const { attr, children } = item
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
          onmouseover: () => selectRange([
            layer.range[0] + attr.range[0],
            layer.range[0] + attr.range[1]
          ]),
          onmouseout: () => selectRange(),
          oncontextmenu: (event) => {
            const filter = filterExpression(attr)
            genet.menu.showContextMenu(event, [
              {
                label: `Apply Filter: ${filter}`,
                click: () => genet.action
                  .emit('core:filter:set', filter),
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
