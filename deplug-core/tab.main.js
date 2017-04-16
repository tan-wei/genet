import GlobalChannel from './global-channel'
import deplug from './deplug'
import jquery from 'jquery'
import mithril from 'mithril'
import path from 'path'
import { remote } from 'electron'
import roll from './roll'

const { webContents, } = remote
async function updateTheme (theme, styleTag, lessFile) {
  const style = await theme.render(lessFile)
  styleTag.text(style.css)
}

export default async function (argv, tab) {
  try {
    const { Theme, Plugin, } = await deplug(argv)
    const { rootDir, } = tab.tab

    const less = tab.tab.less || ''
    if (less !== '') {
      const lessFile = path.join(rootDir, less)
      const styleTag = jquery('<style>').appendTo(jquery('head'))
      GlobalChannel.on('core:theme:updated', () => {
        updateTheme(Theme.current, styleTag, lessFile)
      })
      await updateTheme(Theme.current, styleTag, lessFile)
    }

    const root = tab.tab.root || ''
    if (root !== '') {
      const rootFile = path.join(rootDir, root)
      const func = await roll(rootFile, rootDir)
      const module = {}
      func(module, rootDir)
      mithril.route.prefix('')
      mithril.route(document.body, '/#!', {
        '/': {
          onmatch: (args, requestedPath) => {
            const route = requestedPath.replace(/^\/#!/, '') || '/'
            if (route in module.exports) {
              return module.exports[route]
            }
            return module.exports
          },
        },
      })
    }

    await new Promise((res) => {
      jquery(res)
    })

    await Plugin.loadComponents('panel')

  } catch (err) {
    remote.getCurrentWebContents().openDevTools()
    // eslint-disable-next-line no-console
    console.error(err)
  } finally {
    for (const wc of webContents.getAllWebContents()) {
      wc.send('tab-deplug-loaded', tab.id)
    }
  }
}
