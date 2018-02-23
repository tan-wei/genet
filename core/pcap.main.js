import Content from './lib/content'
import PcapView from './view/pcap/view'

const components = [
  'core:style',
  'core:token',
  'core:panel',
  'core:dissector:packet',
  'core:dissector:stream',
  'core:renderer:attr',
  'core:renderer:layer',
  'core:filter:macro',
  'core:file:importer',
  'core:file:exporter'
]

const content = new Content(
  PcapView,
  'pcap.main.css',
  [
    `--components=${components.join(',')}`,
    '--loggerDomain=pcap',
    '--contextMenu'
  ])
content.load()
