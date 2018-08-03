import displayMain from './display.main'
import packageMain from './package.main'
import preferenceMain from './preference.main'
import windowMain from './window.main'

process.env.GENET_TARGET = process.env.NODE_ENV === 'production'
  ? 'release'
  : 'debug'

const { currentScript } = document
const view = currentScript ? currentScript.getAttribute('data-view') : ''
switch (view) {
  case 'display':
    displayMain()
    break
  case 'package':
    packageMain()
    break
  case 'preference':
    preferenceMain()
    break
  case 'window':
    windowMain()
    break
  default:
    throw new Error(`undefined view: ${view}`)
}
