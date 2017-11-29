import Content from './lib/content'
import PreferenceView from './view/preference/preference-view'

const components = [
  'core:style'
]

const content = new Content(PreferenceView,
  'preference.less',
  [
    `--components=${components.join(',')}`,
    '--loggerDomain=preference',
    '--contextMenu'
  ])
content.load()
