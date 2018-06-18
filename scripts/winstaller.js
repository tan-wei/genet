require('electron-winstaller').createWindowsInstaller({
  appDirectory: 'out/Genet-win32-x64',
  outputDirectory: 'out',
  authors: 'h2so5',
  iconUrl: 'https://raw.githubusercontent.com/deplug/images/master/deplug.ico',
  setupIcon: 'images/deplug-drive.ico',
  loadingGif: 'images/deplug-install.gif',
  noMsi: true,
  exe: 'Genet.exe'
})
