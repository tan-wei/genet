import Content from './lib/content'
import PackageView from './view/package/view'

const components = [
  'core:style'
]

const content = new Content(PackageView,
  'package.main.css',
  [
    `--components=${components.join(',')}`,
    '--loggerDomain=package',
    '--contextMenu'
  ])
content.load()
