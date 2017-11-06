import { app, BrowserWindow, ipcMain } from 'electron'
import WindowFactory from './lib/window-factory'
import env from './lib/env'
import mkpath from 'mkpath'


if (require('electron-squirrel-startup')) {
  app.quit()
}

mkpath.sync(env.userPackagePath)
app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

app.on('ready', () => {
  WindowFactory.create(process.argv.slice(2))
})

ipcMain.on('core:window:loaded', (event, id) => {
  BrowserWindow.fromId(id).show()
})
