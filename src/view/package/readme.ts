import m from 'mithril'
import path from 'path'
import fs from 'fs'
import marked from 'marked'

export default class ReadmeView {
    private file: string
    update(vnode) {
        const file = path.join(vnode.attrs.dir, 'README.md')
        if (this.file != file) {
            this.file = file
            fs.readFile(file, 'utf8', (err, data) => {
                vnode.dom.innerHTML = marked(data || '')
            })
        }
    }

    oncreate(vnode) {
        this.update(vnode)
    }

    onupdate(vnode) {
        this.update(vnode)
    }

    view() {
        return m('div', [])
    }
}
