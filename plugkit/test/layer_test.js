const assert = require('assert')
const {Testing, Token} = require('../test')

describe('Layer', () => {
  describe('#addAttr()', () => {
    it('should throw for wrong arguments', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addAttr(),   TypeError)
      assert.throws(() => layer.addAttr([]), TypeError)
    })
    it('should return Attr', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('Attr', layer.addAttr('.dst').constructor.name)
    })
  })
  describe('#attr()', () => {
    it('should throw for wrong arguments', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.attr(),   TypeError)
      assert.throws(() => layer.attr([]), TypeError)
    })
    it('should return null for unknown id', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal(null, layer.attr('zzz'))
      assert.equal(null, layer.attr(123))
    })
  })
  describe('#id', () => {
    it('should return layer id', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('eth', layer.id)
    })
  })
  describe('#worker', () => {
    it('should return layer worker', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal(0, layer.worker)
      layer.worker = 15
      assert.equal(15, layer.worker)
    })
  })
  describe('#confidence', () => {
    it('should return layer confidence', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal(3, layer.confidence)
      layer.confidence = 1
      assert.equal(1, layer.confidence)
    })
  })
})
