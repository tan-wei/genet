import { app, BrowserWindow, ipcMain } from 'electron'
import { Pcap } from 'plugkit'
import minimist from 'minimist'

if (process.platform === 'darwin' && !Pcap.permission) {
  require('deplug-helper')()
}

app.commandLine.appendSwitch('js-flags', '--harmony-async-await')
app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

app.on('window-all-closed', () => app.quit())
app.on('ready', () => {
  const options = {
    width: 1200,
    height: 600,
    show: false,
    titleBarStyle: 'hidden-inset',
  }
  const mainWindow = new BrowserWindow(options)
  mainWindow.loadURL(`file://${__dirname}/index.htm`)
  const contents = mainWindow.webContents
  contents.on('crashed', () => mainWindow.reload())
  contents.on('unresponsive', () => mainWindow.reload())
  contents.on('did-finish-load', () => {
    const argv = JSON.stringify(minimist(process.argv.slice(2)))
    const script = `require("./window.main.js")(${argv})`
    contents.executeJavaScript(script)
  })
})

ipcMain.on('window-deplug-loaded', (event, id) => {
  BrowserWindow.fromId(id).show()
})
