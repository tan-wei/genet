const assert = require('assert')
const {Token} = require('../test')

describe('Token', () => {
  describe('#get()', () => {
    it('should return 0 for empty string', () => {
      assert.equal(0, Token.get(''));
    })
    it('should return consistent values', () => {
      assert.equal(Token.get('ipv4'), Token.get('ipv4'))
      assert.equal(Token.get('[ipv4]'), Token.get('[ipv4]'))
      assert.equal(Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320'),
        Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320'))
      assert.notEqual(Token.get('ab'), Token.get('ba'))
    })
  })
  
  describe('#string()', () => {
    it('should return empty string for 0', () => {
      assert.equal('', Token.string(0));
    })
    it('should return consistent values', () => {
      assert.equal(Token.get('ipv4'), Token.get(Token.string(Token.get('ipv4'))))
      assert.equal(Token.get('[ipv4]'), Token.get(Token.string(Token.get('[ipv4]'))))
      assert.equal(Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320'),
        Token.get(Token.string(Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320'))))
      assert.notEqual(Token.get('ab'), Token.get(Token.string(Token.get('ba'))))
    })
  })
})
