import Content from './lib/content'
import PreferenceView from './view/preference/preference-view'

const content = new Content(PreferenceView,
  'preference.less', ['--loggerDomain', 'preference'])
content.load()
