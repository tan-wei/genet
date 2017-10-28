import OptionView from './option-view'
import {
  Profile
} from 'deplug'
import m from 'mithril'
export default class PluginCard {
  disable (pkg) {
    Profile.current.set(`_.disabledPlugins.${pkg.name}`, true)
  }
  enable (pkg) {
    Profile.current.delete(`_.disabledPlugins.${pkg.name}`)
  }
  view (vnode) {
    const { pkg } = vnode.attrs
    const options = vnode.attrs.options || {}
    const enabled =
    !Profile.current.get(`_.disabledPlugins.${pkg.name}`, false)
    const name = pkg.name.replace(/^deplugin-/, '')
    const builtin = !pkg.name.startsWith('deplugin-')
    return m('div', {
      class: 'card',
      enabled,
    }, [
      m('div', { class: 'title' }, [
        m('span', [
          name,
          ' ',
          m('small', ['(', pkg.version, ')'])
        ]),
        m('span', {
          style: {
            display: enabled
            ? 'block'
            : 'none',
          },
        }, [
          m('input', {
            type: 'button',
            value: 'Disable',
            onclick: () => this.disable(pkg),
          })
        ]),
        m('span', {
          style: {
            display: enabled
            ? 'none'
            : 'block',
          },
        }, [
          m('input', {
            type: 'button',
            value: 'Enable',
            onclick: () => this.enable(pkg),
          })
        ]),
        m('span', {
          style: {
            display: vnode.attrs.installed && !builtin
            ? 'block'
            : 'none',
          },
        }, [
          m('input', {
            type: 'button',
            value: 'Unistall',
          })
        ])
      ]),
      m('table', {
        style: {
          display: vnode.attrs.installed &&
          Object.keys(options).length
          ? 'block'
          : 'none',
        },
      }, [
        Object.entries(options).map(([id, opt]) => m('tr', [
          m('td', { 'data-tooltip': `.${id}` }, [opt.title]),
          m('td', [
            m(OptionView, {
              pkg,
              id,
              option: opt,
            }),
            m('small', { class: 'description' }, [opt.description])
          ])
        ]))
      ])
    ])
  }
}
