import m from 'mithril'

export default class Dialog {
  constructor(private view, private attrs: object = {}) {
  }

  async show(options = {}) {
    const opt = {
      width: 500,
      height: 320,
      cancelable: true,
      ...options,
    }
    if (document.querySelector('div.dialog-base') !== null) {
      throw new Error('another dialog is showing')
    }

    const base = document.createElement('div')
    base.className = 'dialog-base'
    document.body.appendChild(base)

    return new Promise((res) => {
      function close() {
        m.mount(base, null)
        base.remove()
      }

      function callback(data) {
        close()
        res(data)
      }

      base.addEventListener('click', (event) => {
        if (event.target === base) {
          if (opt.cancelable) {
            close()
          }
        }
      })

      m.mount(base, {
        view: () => m('div', {
          style: { width: `${opt.width}px` },
          class: 'dialog',
        }, [m(this.view, { ...this.attrs, callback })]),
      })
    })
  }
}
