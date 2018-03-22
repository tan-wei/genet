const assert = require('assert')
const { Testing } = require('..')
describe('Payload', () => {
  describe('#addSlice()', () => {
    it('should throw for wrong arguments', () => {
      const payload = Testing.createPayloadInstance()
      assert.throws(() => payload.addSlice(), TypeError)
      assert.throws(() => payload.addSlice(0), TypeError)
      assert.throws(() => payload.addSlice(new Uint8Array([0])), TypeError)
    })
    it('should accept an externalized Uint8Array', () => {
      const payload = Testing.createPayloadInstance()
      const array = Testing.externalize(new Uint8Array([0]))
      assert.doesNotThrow(() => payload.addSlice(array))
      assert.strictEqual(1, payload.length)
    })
  })
  describe('#addAttr()', () => {
    it('should throw for wrong arguments', () => {
      const payload = Testing.createPayloadInstance()
      assert.throws(() => payload.addAttr(), TypeError)
      assert.throws(() => payload.addAttr([]), TypeError)
    })
    it('should return Attr', () => {
      const payload = Testing.createPayloadInstance()
      const ctx = Testing.createContextInstance()
      assert.strictEqual('Attr',
        payload.addAttr(ctx, 'eth.dst').constructor.name)
    })
  })
  describe('#attr()', () => {
    it('should throw for wrong arguments', () => {
      const payload = Testing.createPayloadInstance()
      assert.throws(() => payload.attr(), TypeError)
      assert.throws(() => payload.attr([]), TypeError)
    })
    it('should return null for unknown id', () => {
      const payload = Testing.createPayloadInstance()
      assert.strictEqual(null, payload.attr('zzz'))
      assert.strictEqual(null, payload.attr(123))
    })
    it('should return found Attr', () => {
      const payload = Testing.createPayloadInstance()
      const ctx = Testing.createContextInstance()
      payload.addAttr(ctx, 'eth.dst')
      assert.strictEqual('eth.dst', payload.attr('eth.dst').id)
    })
  })
  describe('#length', () => {
    it('should return payload length', () => {
      const payload = Testing.createPayloadInstance()
      const array = Testing.externalize(new Uint8Array([1, 2, 3, 4, 5]))
      assert.strictEqual(0, payload.length)
      payload.addSlice(array)
      assert.strictEqual(5, payload.length)
      payload.addSlice(array)
      assert.strictEqual(10, payload.length)
    })
  })
  describe('#slices', () => {
    it('should return payload slices', () => {
      const payload = Testing.createPayloadInstance()
      const array = Testing.externalize(new Uint8Array([1, 2, 3, 4, 5]))
      assert.deepEqual([], payload.slices)
      payload.addSlice(array)
      assert.strictEqual(1, payload.slices.length)
      payload.addSlice(array)
      assert.strictEqual(2, payload.slices.length)
    })
  })
  describe('#attrs', () => {
    it('should return payload attrs', () => {
      const payload = Testing.createPayloadInstance()
      const ctx = Testing.createContextInstance()
      assert.deepEqual([], [...payload.attrs()])
      payload.addAttr(ctx, 'eth.dst')
      assert.strictEqual(1, [...payload.attrs()].length)
      payload.addAttr(ctx, 'eth.src')
      assert.strictEqual(2, [...payload.attrs()].length)
    })
  })
  describe('#type', () => {
    it('should return payload type', () => {
      const payload = Testing.createPayloadInstance()
      assert.strictEqual('', payload.type)
      payload.type = '@type'
      assert.strictEqual('@type', payload.type)
    })
  })
  describe('#range', () => {
    it('should return payload range', () => {
      const payload = Testing.createPayloadInstance()
      assert.deepEqual([0, 0], payload.range)
      payload.range = [123, 456]
      assert.deepEqual([123, 456], payload.range)
    })
  })
  describe('#[]', () => {
    it('should return byte at given index', () => {
      const payload = Testing.createPayloadInstance()
      const array = Testing.externalize(new Uint8Array([1, 2, 3, 4, 5]))
      payload.addSlice(array)
      assert.strictEqual(1, payload[0])
      assert.strictEqual(2, payload[1])
      assert.strictEqual(3, payload[2])
      assert.strictEqual(4, payload[3])
      assert.strictEqual(5, payload[4])
    })
    it('should return undefined when index is out of range', () => {
      const payload = Testing.createPayloadInstance()
      assert.strictEqual(undefined, payload[0])
    })
  })
})
