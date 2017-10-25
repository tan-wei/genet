import ConfigView from './config'
import PcapView from './pcap'
import { Tab } from 'deplug'
import m from 'mithril'

export default class View {
  view () {
    if (Tab.page === '') {
      return m(ConfigView, {})
    }
    return m(PcapView, {})
  }
}
