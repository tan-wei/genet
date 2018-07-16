import Style from '../../lib/style'
import m from 'mithril'
import { shell } from 'electron'

export default class PanelView {
  view () {
    return m('div')
  }
  oncreate (vnode) {
    const { attrs } = vnode.attrs
    this.container = document.createElement('div')
    this.container.classList.add('panel')
    const node = this.container.attachShadow({ mode: 'open' })
    const container = document.createElement('div')
    node.appendChild(container)

    container.addEventListener('dragover', (event) => {
      event.preventDefault()
      return false
    }, false)

    container.addEventListener('drop', (event) => {
      event.preventDefault()
      return false
    }, false)

    container.addEventListener('click', (event) => {
      const isUrl = (/^https?:\/\//).test(event.target.href)
      if (event.target.tagName === 'A' && isUrl) {
        event.preventDefault()
        shell.openExternal(event.target.href)
        event.preventDefault()
      }
    })

    m.mount(container, {
      view () {
        return m(vnode.attrs.component, attrs)
      },
    })

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
