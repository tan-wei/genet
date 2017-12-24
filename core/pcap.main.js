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
  'core:filter:string',
  'core:filter:token',
  'core:filter:ast',
  'core:filter:template',
  'core:file:importer',
  'core:file:exporter'
]

const content = new Content(
  PcapView,
  'pcap.less',
  [
    `--components=${components.join(',')}`,
    '--loggerDomain=pcap',
    '--contextMenu'
  ])
content.load()
