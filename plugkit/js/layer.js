const layer = {}
Reflect.defineProperty(layer, 'ConfError', { value: 0 })
Reflect.defineProperty(layer, 'ConfPossible', { value: 1 })
Reflect.defineProperty(layer, 'ConfProbable', { value: 2 })
Reflect.defineProperty(layer, 'ConfExact', { value: 3 })
exports.Layer = layer
