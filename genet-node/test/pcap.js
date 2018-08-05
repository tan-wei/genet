const { Pcap } = require('..')
const assert = require('assert')
describe('Pcap', function() {
    describe('.devices', function() {
        it('should return an array', function() {
            assert(Array.isArray(Pcap.devices()))
        })
    })
    describe('.permission', function() {
        it('should return a bool value', function() {
            assert.equal(typeof Pcap.permission(), 'boolean')
        })
    })
})