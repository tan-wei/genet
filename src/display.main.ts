import Content from './lib/content'
import PcapView from './view/display/view'

export default function main() {
  const components = [
    'core:style',
    'core:token',
    'core:action',
    'core:panel',
    'core:library',
    'core:file:reader',
    'core:renderer:attr',
    'core:renderer:layer',
    'core:filter:macro'
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
}
