import { remote, shell } from 'electron'
import GlobalChannel from './global-channel'
import deplug from './deplug'
import mithril from 'mithril'
import path from 'path'
import roll from './roll'
const { webContents } = remote

const packageWhiteList = [
  'mithril',
  'moment',
  'buffer',
  'path',
  'mithril',
  'deplug',
  'plugkit',
  'object-path',
  'deplugin',
  'deplug-helper',
  'lodash.throttle'
]
async function updateTheme (theme, styleTag, lessFile) {
  const style = await theme.render(lessFile)
  styleTag.textContent = style.css
}

function handleRedirect (event, url) {
  event.preventDefault()
  const contents = remote.getCurrentWebContents()
  if (url !== contents.getURL()) {
    shell.openExternal(url)
  }
}

async function init (profile, argv, tab) {
  for (const wc of webContents.getAllWebContents()) {
    wc.send('tab-deplug-loaded', tab.id)
  }
  try {
    const { Theme, PluginLoader, Tab } = await deplug(profile, argv)
    const { options, id } = tab
    const { rootDir } = tab.tab
    await Promise.all([
      PluginLoader.loadComponents('core:theme'),
      PluginLoader.loadComponents('core:file')
    ])

    const less = tab.tab.less || ''
    if (less !== '') {
      const lessFile = path.join(rootDir, less)
      const styleTag = document.createElement('style')
      document.head.appendChild(styleTag)
      GlobalChannel.on('core:theme:updated', () => {
        updateTheme(Theme.current, styleTag, lessFile)
      })
      await updateTheme(Theme.current, styleTag, lessFile)
    }

    let page = ''
    Reflect.defineProperty(Tab, 'page', {
      get: () => page,
      set: (value) => {
       page = value
      },
    })
    Reflect.defineProperty(Tab, 'options', { value: options })
    Reflect.defineProperty(Tab, 'id', { value: id })

    await PluginLoader.loadComponents('core:filter:literal')
    await PluginLoader.loadComponents('core:renderer')
    await PluginLoader.loadComponents('core:panel')

    const root = tab.tab.root || ''
    if (root !== '') {
      const rootFile = path.join(rootDir, root)
      const func = await roll(rootFile, rootDir)
      const module = {}
      func(module, rootDir)
      mithril.mount(document.body, module.exports)
    }

    await PluginLoader.loadComponents('core:script')

  } catch (err) {
    remote.getCurrentWebContents().openDevTools()
    // eslint-disable-next-line no-console
    console.error(err)
  } finally {
    const contents = remote.getCurrentWebContents()
    contents.on('will-navigate', handleRedirect)
    contents.on('new-window', handleRedirect)
  }
}

export default function (profile, argv, tab) {
  const nodeRequire = require
  const nodeProcess = process

  const contents = remote.getCurrentWebContents()
  contents.on('dom-ready', () => {

    document.addEventListener('dragover', (event) => {
      event.preventDefault()
      return false
    }, false)

    document.addEventListener('drop', (event) => {
      event.preventDefault()
      return false
    }, false)

    global.require = (name) => {
      if (!packageWhiteList.includes(name)) {
        throw new Error(`Cannot find module '${name}'`)
      }
      return nodeRequire(name)
    }
    global.process = {
      execPath: nodeProcess.execPath,
      platform: nodeProcess.platform,
      pid: nodeProcess.pid,
    }
    init(profile, argv, tab)
  })
}
