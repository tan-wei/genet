import Installer from '../../lib/package-install'
import SchemaInput from '../../lib/schema-input'
import env from '../../lib/env'
import m from 'mithril'
import path from 'path'

let installerCallback = null
async function install (pkg) {
  const shortName = pkg.data.name.replace(/@\w+\//, '')
  const installer = new Installer()
  installer.rustpath = deplug.config.get('_.package.rustpath', '')
  installer.on('output', (chunk) => {
    if (installerCallback !== null) {
      installerCallback(chunk)
    }
  })
  try {
    await installer.install(
      path.join(env.userPackagePath, shortName), pkg.archive)
    deplug.notify.show(
      `package: ${shortName}`, {
        type: 'success',
        title: 'Successfully installed',
      })
  } catch (err) {
    deplug.notify.show(
      err.message, {
        type: 'error',
        title: 'Installation failed',
        ttl: 0,
      })
  }
  deplug.packages.update()
}

class ButtonBoxView {
  view (vnode) {
    const { pkg } = vnode.attrs
    if (pkg.url) {
      return m('span', { class: 'button-box' }, [
        m('input', {
          type: 'button',
          value: 'Install',
          onclick: () => {
            install(pkg)
          },
        })
      ])
    }
    if (pkg.removal) {
      return [
        m('p', ['This package will be removed on the next startup.']),
        m('span', { class: 'button-box' }, [
          m('input', {
            type: 'button',
            value: 'Undo',
            onclick: () => {
              deplug.packages.setUninstallFlag(pkg.data.name, false)
            },
          })
        ])
      ]
    }
    return m('span', { class: 'button-box' }, [
      m('input', {
        type: 'button',
        value: pkg.disabled
          ? 'Enable'
          : 'Disable',
        onclick: () => {
          if (pkg.disabled) {
            deplug.packages.enable(pkg.data.name)
          } else {
            deplug.packages.disable(pkg.data.name)
          }
        },
      }),
      m('input', {
        type: 'button',
        value: 'Uninstall',
        style: {
          display: pkg.builtin
            ? 'none'
            : 'block',
        },
        onclick: () => {
          deplug.packages.setUninstallFlag(pkg.data.name, true)
        },
      })
    ])
  }
}

export default class DetailView {
  constructor () {
    this.output = {}
  }

  view (vnode) {
    const { pkg } = vnode.attrs
    if (pkg === null) {
      return m('p', ['No package selected'])
    }

    const config = Object.entries(deplug.config.schema)
      .filter(([id]) => id.startsWith(`${pkg.data.name}.`))
    return m('article', [
      m('h1', { disabled: pkg.disabled === true }, [pkg.data.name,
        m('span', { class: 'version' },
          [pkg.data.version])]),
      m('p', [pkg.data.description]),
      m(ButtonBoxView, { pkg }),
      m('p', config.map(([id, schema]) => m('section', [
        m('h4', [
          schema.title, m('span', { class: 'schema-path' }, [id])]),
        m(SchemaInput, {
          id,
          schema,
        }),
        m('p', { class: 'description' }, [schema.description])
      ]))),
      m('pre', { class: 'output' }, [
        this.output[pkg.data.name]
      ])
    ])
  }

  onupdate (vnode) {
    const { pkg } = vnode.attrs
    if (pkg !== null) {
      installerCallback = (chunk) => {
        this.output[pkg.data.name] = (this.output[pkg.data.name] || '') + chunk
        m.redraw()
      }
    }
  }
}
