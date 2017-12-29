import { app, BrowserWindow, ipcMain, webContents } from 'electron'
import Cache from './lib/cache'
import PackageManager from './lib/package-manager'
import WindowFactory from './lib/window-factory'

if (require('electron-squirrel-startup')) {
  app.quit()
}

app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

async function init () {
  await PackageManager.init()
  await PackageManager.cleanup()
  await Cache.cleanup()
  WindowFactory.create(process.argv.slice(2))
}

app.on('ready', () => {
  init()
})

ipcMain.on('core:window:loaded', (event, id) => {
  const window = BrowserWindow.fromId(id)
  if (!window.isVisible()) {
    window.show()
  }
})

const logContents = new Map()
ipcMain.on('core:logger:register', (event, windowId, contentId) => {
  logContents.set(windowId, contentId)
})
ipcMain.on('core:logger:message', (event, windowId, message) => {
  const contentId = logContents.get(windowId)
  if (Number.isInteger(contentId)) {
    const content = webContents.fromId(logContents.get(windowId))
    if (content !== null) {
      content.send('core:logger:message', message)
    }
  }
})
