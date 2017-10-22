const assert = require('assert')
const {Testing, Token, Layer} = require('../test')

describe('Layer', () => {
  describe('#constructor()', () => {
    it('should throw error', () => {
      assert.throws(() => new Layer(), TypeError)
    })
  })
  describe('.ConfDecodable', () => {
    it('should return 0', () => {
      assert.strictEqual(0, Layer.ConfDecodable)
    })
  })
  describe('.ConfPossible', () => {
    it('should return 1', () => {
      assert.strictEqual(1, Layer.ConfPossible)
    })
  })
  describe('.ConfProbable', () => {
    it('should return 2', () => {
      assert.strictEqual(2, Layer.ConfProbable)
    })
  })
  describe('.ConfExact', () => {
    it('should return 3', () => {
      assert.strictEqual(3, Layer.ConfExact)
    })
  })
  describe('#addAttr()', () => {
    it('should throw for wrong arguments', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addAttr(ctx),   TypeError)
      assert.throws(() => layer.addAttr(ctx, []), TypeError)
    })
    it('should return Attr', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual('Attr', layer.addAttr(ctx, '.dst').constructor.name)
    })
  })
  describe('#addLayer()', () => {
    it('should throw for wrong arguments', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addLayer(ctx),   TypeError)
      assert.throws(() => layer.addLayer(ctx, []), TypeError)
    })
    it('should return Layer', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual('Layer', layer.addLayer(ctx, 'ipv4').constructor.name)
    })
  })
  describe('#addSubLayer()', () => {
    it('should throw for wrong arguments', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addSubLayer(ctx),   TypeError)
      assert.throws(() => layer.addSubLayer(ctx, []), TypeError)
    })
    it('should return Layer', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual('Layer', layer.addSubLayer(ctx, 'ipv4').constructor.name)
    })
  })
  describe('#addPayload()', () => {
    it('should return addPayload', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual('Payload', layer.addPayload(ctx).constructor.name)
    })
  })
  describe('#addTag()', () => {
    it('should throw for wrong arguments', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addTag(),   TypeError)
      assert.throws(() => layer.addTag([]), TypeError)
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
      assert.strictEqual(null, layer.attr('zzz'))
      assert.strictEqual(null, layer.attr(123))
    })
    it('should return found Attr', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      layer.addAttr(ctx, '.dst')
      assert.strictEqual('.dst', layer.attr('.dst').id)
    })
  })
  describe('#id', () => {
    it('should return layer id', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual('eth', layer.id)
    })
  })
  describe('#worker', () => {
    it('should return layer worker', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual(0, layer.worker)
      layer.worker = 15
      assert.strictEqual(15, layer.worker)
    })
  })
  describe('#confidence', () => {
    it('should return layer confidence', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual(3, layer.confidence)
      layer.confidence = 1
      assert.strictEqual(1, layer.confidence)
    })
  })
  describe('#tags', () => {
    it('should return layer tags', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.deepEqual([], layer.tags)
      layer.addTag('@test')
      assert.deepEqual('@test', layer.tags[0])
    })
  })
  describe('#attrs', () => {
    it('should return layer attrs', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.deepEqual([], layer.attrs)
      layer.addAttr(ctx, '.dst')
      assert.strictEqual(1, layer.attrs.length)
      layer.addAttr(ctx, '.src')
      assert.strictEqual(2, layer.attrs.length)
    })
  })
  describe('#payloads', () => {
    it('should return layer payloads', () => {
      const ctx = Testing.createContextInstance()
      const layer = Testing.createLayerInstance('eth')
      assert.deepEqual([], layer.payloads)
      layer.addPayload(ctx)
      assert.strictEqual(1, layer.payloads.length)
      layer.addPayload(ctx)
      assert.strictEqual(2, layer.payloads.length)
    })
  })
  describe('#parent', () => {
    it('should return parent layer', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual('Layer', layer.parent.constructor.name)
      assert.strictEqual(null, layer.parent.parent)
    })
  })
  describe('#frame', () => {
    it('should return Frame', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.strictEqual('Frame', layer.frame.constructor.name)
    })
  })
})
