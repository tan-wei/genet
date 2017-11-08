import Content from './lib/content'
import PreferenceView from './view/preference/preference-view'

const components = ['token']
const content = new Content(
  PreferenceView, 'preference.less', ['--components', components.join(',')])
content.load()
