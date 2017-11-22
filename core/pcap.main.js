import Content from './lib/content'
import PcapView from './view/pcap/pcap-view'

const components = [
    'core:token',
    'core:dissector:packet',
    'core:dissector:stream',
    'core:renderer:attr',
    'core:renderer:layer',
    'core:filter:transform'
]

const content = new Content(
  PcapView, 'pcap.less', ['--components', components.join(',')])
content.load()
