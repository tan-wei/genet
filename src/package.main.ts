import Content from './lib/content'
import PackageView from './view/package/view'

export default function main() {
  const components = [
    'core:style'
  ]

  const content = new Content(PackageView,
    'package.main.css', {
      components,
      loggerDomain: 'package',
      contextMenu: true
    })
  content.load()
}
