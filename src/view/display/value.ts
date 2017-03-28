import fs from 'fs'
import genet from '@genet/api'
import m from 'mithril'
import moment from 'moment'
const { clipboard, remote } = require('electron')
const { dialog } = remote
class BooleanValueItem {
  view(vnode) {
    const faClass = vnode.attrs.value
      ? 'fa-check-square-o'
      : 'fa-square-o'
    return m('span', [
      m('i', { class: `fa ${faClass}` }),
      ' ',
      vnode.attrs.value
        ? 'true'
        : 'false'
    ])
  }
}
class DateValueItem {
  view(vnode) {
    const ts = moment(vnode.attrs.value)
    const tsString = ts.format('YYYY-MM-DDTHH:mm:ss.SSSZ')
    return m('span', [tsString])
  }
}
export class BufferValueItem {
  view(vnode) {
    const maxLen = 6
    const buffer = Buffer.from(vnode.attrs.value)
    const hex = (
      buffer.length > 0
        ? '0x'
        : '') +
      buffer.slice(0, maxLen).toString('hex') +
      (buffer.length > maxLen
        ? '...'
        : '')
    return m('span', {
      oncontextmenu: (event) => {
        genet.menu.showContextMenu(event, [
          {
            label: 'Copy As',
            submenu: [
              {
                label: 'Hex',
                click: () => {
                  clipboard.writeText(buffer.toString('hex'))
                  genet.notify.show('Copied!')
                },
              },
              {
                label: 'Base64',
                click: () => {
                  clipboard.writeText(buffer.toString('base64'))
                  genet.notify.show('Copied!')
                },
              },
              {
                label: 'JSON Array',
                click: () => {
                  clipboard.writeText(JSON.stringify(Array.from(buffer)))
                  genet.notify.show('Copied!')
                },
              },
              {
                label: 'UTF-8',
                click: () => {
                  clipboard.writeText(buffer.toString('utf8'))
                  genet.notify.show('Copied!')
                },
              }
            ],
          },
          {
            label: 'Save As...',
            click: () => {
              const file = dialog.showSaveDialog()
              if (typeof file !== 'undefined') {
                fs.writeFileSync(file, buffer)
              }
            },
          }
        ])
      },
    }, ['[', buffer.length, ' bytes] ', hex])
  }
}
class ArrayValueItem {
  view(vnode) {
    return m('ul', [vnode.attrs.value.map(
      (value) => m('li', [m(AttributeValueItem, { attr: { value } })]))])
  }
}
class ObjectValueItem {
  view(vnode) {
    const obj = vnode.attrs.value
    return m('ul', [Object.keys(obj).map(
      (key) => m('li', [m('span', { class: 'label' }, [key]),
      m(AttributeValueItem, { attr: { value: obj[key] } })]))])
  }
}
class LayerValueItem {
  view(vnode) {
    const layer = vnode.attrs.value
    if (layer.payloads.length) {
      return m('span', [' [', layer.id, '] Payloads '])
    }
    return m('span', [' [', layer.id, '] '])
  }
}
export class AttributeValueItem {
  view(vnode) {
    const { value } = vnode.attrs.attr
    if (value === null) {
      return m('span')
    } else if (typeof value === 'undefined') {
      return m('span', ['undefined'])
    } else if (typeof value === 'boolean') {
      return m(BooleanValueItem, { value })
    } else if (value instanceof Date) {
      return m(DateValueItem, { value })
    } else if (value instanceof Uint8Array) {
      return m(BufferValueItem, { value })
    } else if (Array.isArray(value)) {
      return m(ArrayValueItem, { value })
    } else if (typeof value === 'object' &&
      value.constructor.name === 'Layer') {
      return m(LayerValueItem, { value })
    } else if (typeof value === 'object' &&
      value !== null &&
      Reflect.getPrototypeOf(value) === Object.prototype) {
      return m(ObjectValueItem, { value })
    }
    const result = (value === null
      ? ''
      : value.toString())
    if (result.length > 1024) {
      return m('span', [
        m('details', [
          m('summary', [result.substr(0, 64), '... (', result.length, ')']),
          result
        ])
      ])
    }
    return m('span', [' ', result, ' '])
  }
}
