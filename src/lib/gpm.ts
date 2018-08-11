import gpm from '@genet/gpm'
import execa from 'execa'
const { EventEmitter } = require('events')

export default class Gpm extends EventEmitter {
    private _installerLog: string
    private _tasks: number

    constructor() {
        super()
        this._installerLog = ''
        this._tasks = 0
    }

    get installerLog() {
        return this._installerLog
    }

    get tasks() {
        return this._tasks
    }

    clearInstallerLog() {
        this._installerLog = ''
    }

    async list() {
        const { stdout } = await execa(process.execPath, [gpm, 'list', '--json'], {
            env: {
                ELECTRON_RUN_AS_NODE: '1'
            },
            stdio: 'pipe'
        })
        return JSON.parse(stdout)
    }

    async install(id) {
        const promise = execa(process.execPath, [gpm, 'install', id], {
            env: {
                ELECTRON_RUN_AS_NODE: '1'
            },
            stdio: 'pipe'
        })
        this._tasks += 1
        promise.then(() => {
            this._tasks -= 1
            this.emit('finish', 'Installed', id)
            this.emit('reload')
            this.emit('update')
        }, () => {
            this._tasks -= 1
            this.emit('error', 'Error', `Installation failed: ${id}`)
            this.emit('reload')
            this.emit('update')
        })
        promise.stdout.on('data', (data) => {
            this._installerLog += data
            this.emit('update')
        })
        promise.stderr.on('data', (data) => {
            this._installerLog += data
            this.emit('update')
        })
        this.emit('update')
        return promise
    }

    async uninstall(id) {
        const promise = execa(process.execPath, [gpm, 'uninstall', id], {
            env: {
                ELECTRON_RUN_AS_NODE: '1'
            },
            stdio: 'pipe'
        })
        this._tasks += 1
        promise.then(() => {
            this._tasks -= 1
            this.emit('finish', 'Uninstalled', id)
            this.emit('reload')
            this.emit('update')
        }, () => {
            this._tasks -= 1
            this.emit('error', 'Error', `Uninstallation failed: ${id}`)
            this.emit('reload')
            this.emit('update')
        })
        this.emit('update')
        return promise
    }
}