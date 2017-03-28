export class Disposable {
    private done: boolean = false
    constructor(private readonly disposer: () => void) {
    }

    dispose() {
        if (!this.done) {
            this.done = true
            this.disposer()
        }
    }
}

export class CompositeDisposable extends Disposable {
    constructor(disposables: Disposable[]) {
        super(() => {
            for (const disp of disposables) {
                disp.dispose()
            }
        })
    }
}