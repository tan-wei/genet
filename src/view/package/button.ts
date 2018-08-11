import genet from '@genet/api'
import m from 'mithril'
export default class ButtonBoxView {
  view(vnode) {
    const { pkg } = vnode.attrs
    return m('span', { class: 'button-box' }, [
      m('input', {
        type: 'button',
        value: pkg.disabled
          ? 'Enable'
          : 'Disable',
        onclick: () => {
          if (pkg.disabled) {
            genet.packages.enable(pkg.id)
          } else {
            genet.packages.disable(pkg.id)
          }
        },
      }),
      m('input', {
        type: 'button',
        value: 'Uninstall',
        style: {
          display: pkg.source === ':builtin:'
            ? 'none'
            : 'block',
        },
        disabled: genet.gpm.tasks > 0,
        onclick: () => {
          genet.gpm.uninstall(pkg.id)
        },
      })
    ])
  }
}
