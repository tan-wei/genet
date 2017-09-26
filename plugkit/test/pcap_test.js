const assert = require('assert')
const {Pcap, Token} = require('../test')

describe('Pcap', () => {
  describe('.devices', () => {
    it('should return available network devices', () => {
      assert.equal(1, Pcap.devices.length)
    })
  })
  describe('.permission', () => {
    it('should return true if the process has enough permission', () => {
      assert.equal(true, Pcap.permission)
    })
  })
})
