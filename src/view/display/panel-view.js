import Style from '../../lib/style'
import m from 'mithril'

export default class PanelView {
  view () {
    return m('div')
  }
  oncreate (vnode) {
    const { attrs } = vnode.attrs
    this.container = document.createElement('div')
    this.container.classList.add('panel')
    const node = this.container.attachShadow({ mode: 'open' })
    m.mount(node, { view: () => m(vnode.attrs.component, attrs) })
    const themeStyleTag = document.createElement('style')
    themeStyleTag.id = 'theme-style'
    node.appendChild(themeStyleTag)
    const globalStyleTag = document.createElement('style')
    globalStyleTag.id = 'global-style'
    node.appendChild(globalStyleTag)
    const customStyleTag = document.createElement('style')
    customStyleTag.id = 'custom-style'
    customStyleTag.textContent = vnode.attrs.style
    node.appendChild(customStyleTag)
    const loader = new Style()
    loader.applyCommon(node)
    vnode.dom.appendChild(this.container)
  }
  onbeforeremove () {
    this.container.remove()
  }
}
