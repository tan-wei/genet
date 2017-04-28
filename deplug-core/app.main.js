import { app, BrowserWindow, ipcMain, webContents } from 'electron'
import { Pcap } from 'plugkit'
import config from './config'
import minimist from 'minimist'
import mkpath from 'mkpath'

if (process.platform === 'darwin' && !Pcap.permission) {
  require('deplug-helper')()
}

mkpath.sync(config.userPluginPath)

app.on('window-all-closed', () => {
  for (const wc of webContents.getAllWebContents()) {
    wc.closeDevTools()
  }
  app.quit()
})
app.on('ready', () => {
  const options = {
    width: 1200,
    height: 600,
    show: false,
    titleBarStyle: 'hidden-inset',
    vibrancy: 'ultra-dark',
  }
  const mainWindow = new BrowserWindow(options)
  mainWindow.loadURL(`file://${__dirname}/index.htm`)
  const contents = mainWindow.webContents
  contents.on('crashed', () => mainWindow.reload())
  contents.on('unresponsive', () => mainWindow.reload())
  contents.on('dom-ready', () => {
    const argv = JSON.stringify(minimist(process.argv.slice(2)))
    const script = `require("./window.main.js")(${argv})`
    contents.executeJavaScript(script)
  })
})

ipcMain.on('window-deplug-loaded', (event, id) => {
  BrowserWindow.fromId(id).show()
})
