const { Testing, Token } = require('..')
const { Suite } = require('benchmark')

const ctx = Testing.createContextInstance()
const ethToken = Token`eth`
const dstToken = Token`eth.dst`
const suite = new Suite()
suite
.add('Layer#addAttr', () => {
  const layer = Testing.createLayerInstance(ethToken)
  layer.addAttr(ctx, dstToken)
})
.on('cycle', (event) => {
  console.log(String(event.target))
})
.on('complete', () => {
  process.exit(0)
})
.run({ async: true })
