import PanelView from './panel-view'
import flatten from 'lodash.flatten'
import genet from '@genet/api'
import m from 'mithril'

export default class InputDialog {
  private input: string
  constructor() {
    this.input = ''
  }

  update(vnode) {
    this.input = vnode.dom.querySelector('[name=input-id]').value
  }

  view(vnode) {
    const { callback } = vnode.attrs
    const panels = genet.workspace.panelLayout['dialog:input'] || []
    const layout = flatten(panels).map((tab) => genet.workspace.panel(tab))
      .filter((panel) => typeof panel !== 'undefined')
    if (this.input === '' && layout.length > 0) {
      this.input = layout[0].id
    }
    return m('div', { class: 'tool-view' }, [
      m('div', {
        style: {
          display: layout.length === 0
            ? 'block'
            : 'none',
        },
      }, [
          m('h1', ['No input source']),
          m('p', ['There is no available input source. Please install related package.'])
        ]),
      m('ul', {
        style: {
          display: layout.length > 0
            ? 'block'
            : 'none',
        },
      }, [
          m('li', [
            m('select', {
              name: 'input-id',
              onchange: () => this.update(vnode),
            }, layout.map((panel) =>
              m('option', { value: panel.id }, [panel.name])))
          ])
        ]),
      m('div',
        layout.map((panel) => m('div', {
          style: {
            display: panel.id === this.input
              ? 'block'
              : 'none',
          },
        }, [
            m(PanelView, Object.assign(panel, { attrs: { callback } }))
          ]))
      )])
  }
}
