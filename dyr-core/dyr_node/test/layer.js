const { Layer } = require('..')
const assert = require('assert')
describe('Layer', function() {
  describe('#constructor()', function() {
    it('should return a Layer', function() {
      assert(new Layer(5) instanceof Layer)
    })
    it('should throw for wrong arguments', () => {
      assert.throws(() => new Layer(), TypeError)
      assert.throws(() => new Layer('aaa'), TypeError)
    })
  })
  describe('#id', function() {
    it('should return the id', function() {
      const layer = new Layer(5)
      assert.equal(layer.id, 5)
    })
  })
  describe('#addChild', function() {
    it('should throw for wrong arguments', () => {
      const layer = new Layer(5)
      assert.throws(() => layer.addChild(), TypeError)
      assert.throws(() => layer.addChild('aaa'), TypeError)
      assert.throws(() => layer.addChild(layer), TypeError)
    })
    it('moves child layer', () => {
      const layer = new Layer(5)
      const child = new Layer(6)
      layer.addChild(child)
      assert.throws(() => child.id, ReferenceError)
    })
  })
})
