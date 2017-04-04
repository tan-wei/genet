'use strict';
const { Channel, Tab } = require('deplug')

class Window extends HTMLElement {
  constructor() {
    super()

    this.tabs = []
    Channel.on('core:create-tab', (_, name) => {
      if (let temp = Tab.getTemplate(name)) {

      }
    })
  }

  connectedCallback() {
    var t = document.currentScript.ownerDocument.querySelector('#sdtemplate')
    var clone = document.importNode(t.content, true)
    this.createShadowRoot().appendChild(clone)
  }
}
customElements.define('deplug-window-main', Window)
