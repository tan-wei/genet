import Content from './lib/content'
import PackageView from './view/package/view'

const components = [
  'core:style'
]

const content = new Content(PackageView,
  'package.less',
  [
    `--components=${components.join(',')}`,
    '--loggerDomain=pacakge',
    '--contextMenu'
  ])
content.load()
