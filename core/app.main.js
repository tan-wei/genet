import { app, BrowserWindow, ipcMain, webContents } from 'electron'
import Cache from './lib/cache'
import WindowFactory from './lib/window-factory'
import env from './lib/env'
import mkpath from 'mkpath'

if (require('electron-squirrel-startup')) {
  app.quit()
}

mkpath.sync(env.userPackagePath)
mkpath.sync(env.cachePath)
Cache.cleanup()

app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

app.on('ready', () => {
  WindowFactory.create(process.argv.slice(2))
})

ipcMain.on('core:window:loaded', (event, id) => {
  BrowserWindow.fromId(id).show()
})

const logContents = new Map()
ipcMain.on('core:logger:register', (event, windowId, contentId) => {
  logContents.set(windowId, contentId)
})
ipcMain.on('core:logger:message', (event, windowId, message) => {
  const contentId = logContents.get(windowId)
  if (Number.isInteger(contentId)) {
    webContents.fromId(contentId).send('core:logger:message', message)
  }
})
