import genet from '@genet/api'
import m from 'mithril'

export default class InstallView {
    view(vnode) {
        return m('article', [
            m('h1', [m('i', { class: 'fa fa-gift' }), ' ', 'Install Package']),
            m('p', [
                m('span', { class: 'button-box' }, [
                    m('input', {
                        type: 'text',
                        placeholder: 'Package Source...',
                        name: 'source'
                    }),
                    m('input', {
                        type: 'button',
                        value: 'Install',
                        disabled: genet.gpm.tasks > 0,
                        onclick: () => {
                            const source = vnode.dom.querySelector('[name=source]').value.trim()
                            if (source.length > 0) {
                                genet.notify.show(`Package from ${source}`, {
                                    title: 'Installing...'
                                })
                                genet.gpm.install(source)
                            }
                        }
                    })
                ]),
                m('h4', ['Available Sources:']),
                m('ul', [
                    m('li', ['Local Path: e.g. ', m('b', ['/home/genet/package'])]),
                    m('li', ['Remote Git Repository: e.g. ', m('b', ['https://github.com/genet-app/arp'])])
                ])
            ]),
            m('hr'),
            m('pre', [genet.gpm.installerLog])
        ])
    }
}
