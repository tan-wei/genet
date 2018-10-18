import Content from './lib/content'
import PcapView from './view/display/view'

export default function main() {
  const components = [
    'core:style',
    'core:token',
    'core:panel',
    'core:library',
    'core:file:reader',
    'core:renderer:attr',
    'core:renderer:layer'
  ]

  const content = new Content(
    PcapView,
    'display.main.css',
    {
      components,
      loggerDomain: 'display',
      contextMenu: true
    })
  content.load()
}
