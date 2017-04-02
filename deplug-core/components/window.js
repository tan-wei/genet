import Component from './base'
import Theme from '../theme'
import jquery from 'jquery'
import objpath from 'object-path'
import path from 'path'

export default class WindowComponent extends Component {
  async load () {
    const less = objpath.get(this.comp, 'window.less', '')
    if (less !== '') {
      const lessFile = path.join(path.dirname(this.rootPath), less)
      const style = await Theme.current.render(lessFile)
      jquery('head').append(jquery('<style>').text(style.css))
    }
  }
}
