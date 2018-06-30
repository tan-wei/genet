import m from 'mithril'

export default class ToolBar {
  view (vnode) {
    const { viewState, sess } = vnode.attrs
    return m('div', { class: 'toolbar' }, [
      m('span', {
        class: 'button',
        'data-balloon': `Capture ${viewState.capture
          ? 'Running'
          : 'Paused'}`,
        'data-balloon-pos': 'right',
        onclick: () => {
          if (viewState.capture) {
            sess.stopStream()
          } else {
            sess.startStream()
          }
          viewState.capture = !viewState.capture
        },
      }, [
        m('i', {
          class: viewState.capture
            ? 'fa fa-play-circle'
            : 'fa fa-pause-circle',
        })
      ]),
      m('span', {
        class: 'button',
        'data-balloon': `Scroll ${viewState.scrollLock
          ? 'Locked'
          : 'Unlocked'}`,
        'data-balloon-pos': 'right',
        onclick: () => {
          viewState.scrollLock = !viewState.scrollLock
        },
      }, [
        m('i', {
          class: viewState.scrollLock
            ? 'fa fa-lock'
            : 'fa fa-unlock-alt',
        })
      ]),
      m('span', {
        'data-balloon': 'Frame Counter',
        'data-balloon-pos': 'right',
      }, [viewState.counter])
    ])
  }
}
