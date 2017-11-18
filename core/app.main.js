import { app, BrowserWindow, ipcMain, webContents } from 'electron'
import Cache from './lib/cache'
import PackageManager from './lib/package-manager'
import WindowFactory from './lib/window-factory'
import env from './lib/env'
import mkpath from 'mkpath'

if (require('electron-squirrel-startup')) {
  app.quit()
}

app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

async function init () {
  await mkpath(env.userPackagePath)
  await mkpath(env.cachePath)
  await Cache.cleanup()
  await PackageManager.cleanup()
  WindowFactory.create(process.argv.slice(2))
}

app.on('ready', () => {
  init()
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
