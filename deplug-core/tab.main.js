import { remote } from 'electron'
import deplug from './deplug'
import jquery from 'jquery'
import path from 'path'

export default async function (argv, tab) {
  try {
    const { Theme, } = await deplug(argv)

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
  }
}
