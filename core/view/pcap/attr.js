import { AttributeValueItem } from './value'
import m from 'mithril'

function selectRange (range = []) {
  deplug.action.emit('core:frame:range-selected', range)
}

function filterExpression (attr) {
  const macro = deplug.session.attrMacro(attr.type)
  if (macro !== null) {
    const exp = macro(attr)
    const prefix = deplug.config.get('_.filter.macroPrefix', '@')
    return `${attr.id} == ${prefix}${exp}`
  }
  if (attr.value === true) {
    return attr.id
  } else if (attr.value === false) {
    return `!${attr.id}`
  } else if (attr.value instanceof Uint8Array) {
    return `${attr.id} == ${JSON.stringify(Array.from(attr.value))}`
  }
  return `${attr.id} == ${JSON.stringify(attr.value)}`
}

export default class AttributeItem {
  view (vnode) {
    const { item, layer } = vnode.attrs
    const { attr, children } = item
    let faClass = 'attribute'
    if (children.length) {
      faClass = 'attribute children'
    }
    const { name } = deplug.session.token(attr.id)
    const attrRenderer =
      deplug.session.attrRenderer(attr.type) || AttributeValueItem
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
            deplug.menu.showContextMenu(event, [
              {
                label: `Apply Filter: ${filter}`,
                click: () => deplug.action
                  .emit('core:filter:set', filter),
              },
              {
                label: 'Reveal in Developer Tools...',
                click: () => {
                  // eslint-disable-next-line no-console
                  console.log(attr)
                  deplug.action.global.emit('core:tab:open-devtool')
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
