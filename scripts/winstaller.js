require('electron-winstaller').createWindowsInstaller({
  appDirectory: 'out/Genet-win32-x64',
  outputDirectory: 'out',
  authors: 'h2so5',
  iconUrl: 'https://raw.githubusercontent.com/genet-app/images/master/genet.ico',
  setupIcon: 'images/genet-drive.ico',
  loadingGif: 'images/genet-install.gif',
  noMsi: true,
  exe: 'Genet.exe'
})
