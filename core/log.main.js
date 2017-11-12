import Content from './lib/content'
import LogView from './view/log/log-view'

const content = new Content(LogView, 'preference.less')
content.load()
