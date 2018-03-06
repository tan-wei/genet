const assert = require('assert')
const { Token } = require('..')
describe('Token', () => {
  it('should return 0 for empty string', () => {
    assert.strictEqual(0, Token``)
  })
  it('should return consistent values', () => {
    assert.strictEqual(Token`ipv4`, Token`ipv4`)
    assert.strictEqual(Token`[ipv4]`, Token`[ipv4]`)
    assert.strictEqual(Token`9bbb35e6-cde1-419b-8d7b-ea04c87e7320`,
      Token`9bbb35e6-cde1-419b-8d7b-ea04c87e7320`)
    assert.notEqual(Token`ab`, Token`ba`)
  })
  describe('#get()', () => {
    it('should throw for wrong arguments', () => {
      assert.throws(() => Token.get(0), TypeError)
    })
    it('should return 0 for empty string', () => {
      assert.strictEqual(0, Token.get())
      assert.strictEqual(0, Token.get(''))
    })
    it('should return consistent values', () => {
      assert.strictEqual(Token.get('ipv4'), Token.get('ipv4'))
      assert.strictEqual(Token.get('[ipv4]'), Token.get('[ipv4]'))
      assert.strictEqual(Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320'),
        Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320'))
      assert.notEqual(Token.get('ab'), Token.get('ba'))
    })
  })
  describe('#string()', () => {
    it('should throw for wrong arguments', () => {
      assert.throws(() => Token.string(), TypeError)
      assert.throws(() => Token.string('ipv4'), TypeError)
    })
    it('should return empty string for 0', () => {
      assert.strictEqual('', Token.string(0))
    })
    it('should return consistent values', () => {
      assert.strictEqual(Token.get('ipv4'),
        Token.get(Token.string(Token.get('ipv4'))))
      assert.strictEqual(Token.get('[ipv4]'),
        Token.get(Token.string(Token.get('[ipv4]'))))
      assert.strictEqual(Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320'),
        Token.get(Token.string(
          Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320'))))
      assert.notEqual(Token.get('ab'), Token.get(Token.string(Token.get('ba'))))
    })
  })

  describe('#concat()', () => {
    it('should throw for wrong arguments', () => {
      assert.throws(() => Token.concat(), TypeError)
      assert.throws(() => Token.concat([]), TypeError)
      assert.throws(() => Token.concat(0, []), TypeError)
      assert.throws(() => Token.concat('ipv4', 1), TypeError)
    })
    it('should return a joined token', () => {
      assert.strictEqual(Token.concat('', ''), Token.get())
      assert.strictEqual(Token.concat(Token.get('eth'), '.dst'),
        Token.get('eth.dst'))
      assert.strictEqual(Token.concat('eth', ''),
        Token.get('eth'))
      assert.strictEqual(Token.concat(Token.get('0f73db21-11f6-'),
        '4987-81f6-1a8bb5f26704'),
      Token.get('0f73db21-11f6-4987-81f6-1a8bb5f26704'))
    })
  })
})
