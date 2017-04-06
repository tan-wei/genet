import deplug from './deplug'
import jquery from 'jquery'
import path from 'path'
import { remote } from 'electron'

const { webContents, } = remote
export default async function (argv, tab) {
  try {
    const { Theme, } = await deplug(argv)

    const less = tab.tab.less || ''
    if (less !== '') {
      const lessFile = path.join(tab.tab.rootDir, less)
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
      wc.send('tab-deplug-loaded', tab.id)
    }
  }
}
