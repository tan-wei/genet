import { app, BrowserWindow } from 'electron';
import { Pcap } from 'plugkit';
import Window from './window';

app.commandLine.appendSwitch('js-flags', '--harmony-async-await')
app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

app.on('window-all-closed', () => app.quit())
app.on('ready', () => {
  let options = {
    width: 1200,
    height: 600,
    show: true,
    titleBarStyle: 'hidden-inset'
  }
  let mainWindow = new BrowserWindow(options)
  mainWindow.loadURL(`file://${__dirname}/index.htm`)
  mainWindow.webContents.on('crashed', () => mainWindow.reload())
  mainWindow.webContents.on('unresponsive', () => mainWindow.reload())
})
