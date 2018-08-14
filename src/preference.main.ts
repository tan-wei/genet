import Content from './lib/content'
import PreferenceView from './view/preference/view'

export default function main() {
  const components = [
    'core:style'
  ]

  const content = new Content(PreferenceView,
    'preference.main.css', {
      components,
      loggerDomain: 'preference',
      contextMenu: true
    })
  content.load()
}
