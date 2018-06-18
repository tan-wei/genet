import { app, BrowserWindow, ipcMain, Menu, webContents } from 'electron'
import Config from './config'
import debounce from 'lodash.debounce'
import env from './env'
import flatten from 'flat'
import path from 'path'
import url from 'url'

app.on('window-all-closed', () => {
  for (const wc of webContents.getAllWebContents()) {
    wc.closeDevTools()
  }
  app.quit()
})

export default class WindowFactory {
  static create (argv, profile = 'default') {
    const conf = new Config(profile, 'workspace')
    const size = conf.get('_.window.size')
    const options = {
      width: size[0],
      height: size[1],
      titleBarStyle: conf.get('_.window.titleBarStyle'),
      show: false,
    }
    if (process.platform === 'linux') {
      options.icon = env.linuxIconPath
    }
    const mainWindow = new BrowserWindow(options)

    const jsonArgv =
      JSON.stringify(argv.concat([`--profile=${profile}`]))

    const localUrl = url.format({
      protocol: 'file',
      slashes: true,
      pathname: path.join(__dirname, 'asset', 'window.htm'),
      search: jsonArgv,
    })
    mainWindow.loadURL(localUrl)

    mainWindow.on('resize', debounce(() => {
      conf.set('_.window.size', mainWindow.getSize())
    }), 500)

    const contents = mainWindow.webContents
    contents.on('crashed', () => mainWindow.reload())
    contents.on('unresponsive', () => mainWindow.reload())

    function reloadMenu () {
      const script = 'genet.menu.template'
      contents.executeJavaScript(script).then((template) => {
        const flatMenu = flatten({ template })
        for (const [key, channel] of Object.entries(flatMenu)) {
          if (key.endsWith('.action')) {
            flatMenu[key.replace('.action', '.click')] = () => {
              mainWindow.webContents
                .send('core:menu:action', channel)
            }
          }
        }
        const menu =
          Menu.buildFromTemplate(flatten.unflatten(flatMenu).template)
        if (process.platform === 'darwin') {
          Menu.setApplicationMenu(menu)
        } else {
          mainWindow.setMenu(menu)
        }
      })
    }

    mainWindow.on('focus', reloadMenu)

    ipcMain.on('core:menu:reload', (event, id) => {
      if (id === mainWindow.id) {
        reloadMenu()
      }
    })

    return mainWindow
  }
}
