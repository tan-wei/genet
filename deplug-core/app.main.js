import { app, BrowserWindow } from 'electron'
import { Pcap } from 'plugkit'
import minimist from 'minimist'

if (process.platform === 'darwin' && !Pcap.permission) {
  require('deplug-helper')()
}

app.commandLine.appendSwitch('js-flags', '--harmony-async-await')
app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

app.on('window-all-closed', () => app.quit())
app.on('ready', () => {
  let options = {
    width: 1200,
    height: 600,
    show: false,
    titleBarStyle: 'hidden-inset',
  }
  let mainWindow = new BrowserWindow(options)
  mainWindow.loadURL(`file://${__dirname}/index.htm`)
  let contents = mainWindow.webContents
  contents.on('crashed', () => mainWindow.reload())
  contents.on('unresponsive', () => mainWindow.reload())
  contents.on('did-finish-load', () => {
    let argv = JSON.stringify(minimist(process.argv.slice(2)))
    let script = `require("./window.main.js")(${argv})`
    contents.executeJavaScript(script, false).then(() => {
      mainWindow.show()
    })
  })
})
