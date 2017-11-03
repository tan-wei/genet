import { ipcRenderer, remote } from 'electron'
import Deplug from './deplug'
import RootView from './root-view'
import ThemeLoader from './theme-loader'
import m from 'mithril'
import minimist from 'minimist'
import ready from 'document-ready-promise'

export default async function (argv) {
  const options = minimist(argv)
  Reflect.defineProperty(window, 'deplug', { value: new Deplug(options) })
  const loader = new ThemeLoader(`${__dirname}/theme.less`)
  await loader.load(`${__dirname}/window.less`, document.head)
  m.mount(document.body, RootView)
  ipcRenderer.send('core:window:loaded', remote.getCurrentWindow().id)
  await ready()
}

document.addEventListener('dragover', (event) => {
  event.preventDefault()
  return false
}, false)

document.addEventListener('drop', (event) => {
  event.preventDefault()
  return false
}, false)
