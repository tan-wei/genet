import { app, BrowserWindow, ipcMain } from 'electron'
import WindowFactory from './window-factory'
import config from './config'
import mkpath from 'mkpath'


if (require('electron-squirrel-startup')) {
  app.quit()
}

mkpath.sync(config.userPluginPath)
app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

app.on('ready', () => {
  WindowFactory.create(process.argv.slice(2))
})

ipcMain.on('core:window:loaded', (event, id) => {
  BrowserWindow.fromId(id).show()
})
