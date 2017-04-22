import {remote} from 'electron'

export default class Menu {
  action() {
    remote.getCurrentWindow().toggleDevTools()
  }
}
