import Content from './lib/content'
import PcapView from './view/display/view'

const components = [
  'core:style',
  'core:token',
  'core:panel',
  'core:library',
  'core:file:reader',
  'core:renderer:attr',
  'core:renderer:layer',
  'core:filter:macro',
  'core:file:importer',
  'core:file:exporter'
]

const content = new Content(
  PcapView,
  'display.main.css',
  [
    `--components=${components.join(',')}`,
    '--loggerDomain=display',
    '--contextMenu'
  ])
content.load()
