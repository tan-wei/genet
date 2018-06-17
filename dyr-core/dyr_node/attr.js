class Attr {
    constructor(ctx, layer, name, type, get, len) {
        const getFunc = (typeof get === 'function')
            ? get
            : new Function(`return ${JSON.stringify(get)}`)
        const lenFunc = (typeof len === 'function')
            ? len
            : new Function(`return ${JSON.stringify(len)}`)
        this._fields = {
            name, type, get, len
        }
    }

    get name() {
        return this._fields.name
    }

    get type() {
        return this._fields.type
    }

    get(data) {
        return this._fields.get(data)
    }
    
    len(data) {
        return this._fields.len(data)
    }
}

module.exports = Attr
