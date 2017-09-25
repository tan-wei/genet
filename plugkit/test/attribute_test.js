const assert = require('assert')
const {Testing, Token} = require('../test')

describe('Attr', () => {
  describe('#id', () => {
    it('should return attribute id', () => {
      const attr = Testing.createAttrInstance('eth')
      assert.equal('eth', attr.id)
    })
  })
  describe('#value', () => {
    it('should return attribute value', () => {
      const token = Token.get('eth')
      const attr = Testing.createAttrInstance(token)
      assert.deepEqual(null, attr.value)
      attr.value = true
      assert.deepEqual(true, attr.value)
      attr.value = -0.5
      assert.deepEqual(-0.5, attr.value)
      attr.value = "hello"
      assert.deepEqual("hello", attr.value)
      attr.value = {a: '0'}
      assert.deepEqual({a: '0'}, attr.value)
      attr.value = [123, 456]
      assert.deepEqual([123, 456], attr.value)
    })
  })
  describe('#range', () => {
    it('should return attribute range', () => {
      const token = Token.get('eth')
      const attr = Testing.createAttrInstance(token)
      assert.deepEqual([0, 0], attr.range)
      attr.range = [123, 456]
      assert.deepEqual([123, 456], attr.range)
    })
  })
  describe('#type', () => {
    it('should return attribute type', () => {
      const token = Token.get('eth')
      const attr = Testing.createAttrInstance(token)
      assert.equal('', attr.type)
      attr.type = '@type'
      assert.deepEqual('@type', attr.type)
    })
  })
})
