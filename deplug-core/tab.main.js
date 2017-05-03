import { remote, shell } from 'electron'
import GlobalChannel from './global-channel'
import deplug from './deplug'
import jquery from 'jquery'
import mithril from 'mithril'
import path from 'path'
import roll from './roll'

const { webContents } = remote
async function updateTheme (theme, styleTag, lessFile) {
  const style = await theme.render(lessFile)
  styleTag.text(style.css)
}

function handleRedirect (event, url) {
  event.preventDefault()
  const contents = remote.getCurrentWebContents()
  if (url !== contents.getURL()) {
    shell.openExternal(url)
  }
}

export default async function (argv, tab) {
  try {
    const { Theme, Plugin, Tab } = await deplug(argv)
    const { options, id } = tab
    const { rootDir } = tab.tab

    const less = tab.tab.less || ''
    if (less !== '') {
      const lessFile = path.join(rootDir, less)
      const styleTag = jquery('<style>').appendTo(jquery('head'))
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

    await Plugin.loadComponents('panel')

    const root = tab.tab.root || ''
    if (root !== '') {
      const rootFile = path.join(rootDir, root)
      const func = await roll(rootFile, rootDir)
      const module = {}
      func(module, rootDir)
      mithril.mount(document.body, module.exports)
    }

    await new Promise((res) => {
      jquery(res)
    })

    await Plugin.loadComponents('script')

  } catch (err) {
    remote.getCurrentWebContents().openDevTools()
    // eslint-disable-next-line no-console
    console.error(err)
  } finally {
    for (const wc of webContents.getAllWebContents()) {
      wc.send('tab-deplug-loaded', tab.id)
    }

    const contents = remote.getCurrentWebContents()
    contents.on('will-navigate', handleRedirect)
    contents.on('new-window', handleRedirect)
  }
}
