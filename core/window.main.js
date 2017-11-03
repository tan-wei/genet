import { ipcRenderer, remote } from 'electron'
import Deplug from './deplug'
import RootView from './root-view'
import ThemeLoader from './theme-loader'
import m from 'mithril'

async function load () {
  const loader = new ThemeLoader(`${__dirname}/theme.less`)
  await loader.load(`${__dirname}/window.less`, document.head)
  m.mount(document.body, RootView)
  await document.fonts.ready
  ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
}

document.addEventListener('dragover', (event) => {
  event.preventDefault()
  return false
}, false)

document.addEventListener('drop', (event) => {
  event.preventDefault()
  return false
}, false)

const argv = JSON.parse(decodeURIComponent(location.search.substr(1)))
Reflect.defineProperty(window, 'deplug', { value: new Deplug(argv) })

load()
