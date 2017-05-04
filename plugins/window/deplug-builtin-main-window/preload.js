const args = JSON.parse(navigator.userAgent)
navigator.userAgent = ''
require('deplug-core/tab.new.main.js')(...args)
