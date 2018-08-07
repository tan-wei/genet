import m from 'mithril'
import path from 'path'
import url from 'url'

export default class WebView {
  view(vnode) {
    const { tab } = vnode.attrs
    const localUrl = url.format({
      protocol: 'file',
      slashes: true,
      pathname: path.join(__dirname, tab.src),
      search: JSON.stringify(tab.argv),
    })
    return m('webview', {
      key: tab.id,
      src: localUrl,
      nodeintegration: true,
      active: vnode.attrs.active,
    })
  }

  oncreate(vnode) {
    vnode.dom.setAttribute('loading', '')
    vnode.dom.addEventListener('crashed', () => {
      const localUrl = url.format({
        protocol: 'file',
        slashes: true,
        pathname: path.join(__dirname, '..', 'asset', 'crashed.htm'),
        search: Buffer.from(vnode.dom.getURL(), 'utf8').toString('base64'),
      })
      vnode.dom.loadURL(localUrl)
    })
    vnode.dom.addEventListener('did-finish-load', () => {
      vnode.dom.removeAttribute('loading')
    })
  }
}
