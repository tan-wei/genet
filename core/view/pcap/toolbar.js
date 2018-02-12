import m from 'mithril'

export default class ToolBar {
  view (vnode) {
    const { counter, capture, sess } = vnode.attrs
    return m('div', { class: 'toolbar' }, [
      m('span', {
        class: 'button',
        'data-balloon': `Capture ${capture
          ? 'Running'
          : 'Paused'}`,
        'data-balloon-pos': 'right',
        onclick: () => {
          if (capture) {
            sess.stopPcap()
          } else {
            sess.startPcap()
          }
          this.capture = !this.capture
        },
      }, [
        m('i', {
          class: this.capture
            ? 'fa fa-play-circle'
            : 'fa fa-pause-circle',
        })
      ]),
      m('span', {
        class: 'button',
        'data-balloon': `Scroll ${this.scrollLock
          ? 'Locked'
          : 'Unlocked'}`,
        'data-balloon-pos': 'right',
        onclick: () => {
          this.scrollLock = !this.scrollLock
        },
      }, [
        m('i', {
          class: this.scrollLock
            ? 'fa fa-lock'
            : 'fa fa-unlock-alt',
        })
      ]),
      m('span', {
        'data-balloon': 'Frame Counter',
        'data-balloon-pos': 'right',
      }, [counter])
    ])
  }
}
