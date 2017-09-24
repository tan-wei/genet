const assert = require('assert')
const {Testing, Token} = require('../test')

describe('Attr', () => {
  describe('#id', () => {
    it('should return attribute id', () => {
      const token = Token.get('eth')
      const attr = Testing.createAttrInstance(token)
      assert.equal(token, attr.id)
    })
  })
  describe('#value', () => {
    it('should return attribute value', () => {
      const token = Token.get('eth')
      const attr = Testing.createAttrInstance(token)
      assert.equal(null, attr.value)
    })
  })
})
