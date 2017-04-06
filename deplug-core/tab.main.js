import { ipcRenderer, remote } from 'electron'
const { webContents, } = remote
import deplug from './deplug'
import jquery from 'jquery'
import path from 'path'

export default async function (argv, tab, id) {
  try {
    const { Theme, Channel } = await deplug(argv)

    const less = tab.less || ''
    if (less !== '') {
      const lessFile = path.join(tab.rootDir, less)
      const style = await Theme.current.render(lessFile)
      jquery('head').append(jquery('<style>').text(style.css))
    }

    await new Promise((res) => {
      jquery(res)
    })
  } catch (err) {
    remote.getCurrentWebContents().openDevTools()
    throw err
  } finally {
    for (const wc of webContents.getAllWebContents()) {
      wc.send('tab-deplug-loaded', id)
    }
  }
}
