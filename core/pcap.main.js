import Content from './lib/content'
import PcapView from './view/pcap/pcap-view'

const components = ['token', 'dissector']
const content = new Content(
  PcapView, 'pcap.less', ['--components', components.join(',')])
content.load()
