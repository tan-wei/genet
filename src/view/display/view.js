import { HSplitter, VSplitter } from '../../lib/splitter'
import PcapDetailView from './detail'
import PcapToolView from './tool'
import TopView from './top'
import m from 'mithril'

class MainView {
  view () {
    return m('main', [
      m(HSplitter, {
        right: PcapDetailView,
        left: PcapToolView,
        parent: this,
        width: 350,
        workspace: '_.pcapToolWidth',
      })
    ])
  }
}

export default class PcapView {
  view () {
    return [
      m(VSplitter, {
        top: TopView,
        bottom: MainView,
        parent: this,
        height: 300,
        workspace: '_.pcapBottomHeight',
      }),
      m('div', { class: 'notification' })
    ]
  }
}
