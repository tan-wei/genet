import ButtonBoxView from './button'
import Installer from '../../lib/package-install'
import SchemaInput from '../../lib/schema-input'
import env from '../../lib/env'
import m from 'mithril'
import path from 'path'

let installerCallback = null
async function install (pkg) {
  const shortName = pkg.id
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
      .filter(([id]) => id.startsWith(`${pkg.id}.`))
    return m('article', [
      m('h1', { disabled: pkg.disabled || pkg.incompatible }, [pkg.data.name,
        m('span', { class: 'version' },
          [pkg.data.version])]),
      m('p', [pkg.data.description]),
      m('p', {
        style: {
          color: 'var(--theme-error)',
          display: pkg.incompatible
            ? 'block'
            : 'none',
        },
      }, [
        'This package is incompatible with the running Deplug version.',
        m('br'),
        `Required Deplug Version: ${pkg.data.engines.genet}`
      ]),
      m(ButtonBoxView, {
        pkg,
        install,
      }),
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
        this.output[pkg.id]
      ])
    ])
  }

  onupdate (vnode) {
    const { pkg } = vnode.attrs
    if (pkg !== null) {
      installerCallback = (chunk) => {
        this.output[pkg.id] = (this.output[pkg.id] || '') + chunk
        m.redraw()
      }
    }
  }
}
