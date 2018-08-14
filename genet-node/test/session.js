const { Session } = require('..')
const { Profile } = Session

const assert = require('assert')
describe('Session.Profile', () => {
  describe('#constructor', () => {
        it('should return a Profile', function() {
            assert((new Profile()) instanceof Profile)
        })
    })
  describe('#addDecoder', () => {
        it('should add a decoder', function(done) {
            const profile = new Profile()
            profile.addLinkLayer(1, '[eth]')
            profile.addDecoder(`
        class Diss {
          analyze(ctx, layer) {
            return true
          }
        }
        Diss
      `, { layerHints: ['[eth]'] })
            const sess = new Session(profile)
            sess.pushFrames([{ data: ethData }], 1)
            sess.on('event', (e) => {
                if (e.type === 'frames' && e.length === 1) {
                    sess.close()
                    done()
                }
            })
        })
        it('should throw for wrong arguments', () => {
            const profile = new Profile()
            assert.throws(() => profile.addDecoder(), TypeError)
            assert.throws(() => profile.addDecoder(''), TypeError)
            assert.throws(() => profile.addDecoder('', 0), TypeError)
        })
    })
})

describe('Session', () => {
  describe('#constructor', () => {
        it('should return a Session', function() {
            const profile = new Profile()
            const sess = new Session(profile)
            assert(sess instanceof Session)
            assert.equal(sess.length, 0)
            sess.close()
        })
        it('should throw for wrong arguments', () => {
            assert.throws(() => new Session(), TypeError)
            assert.throws(() => new Session('aaa'), TypeError)
        })
    })
  describe('#close', () => {
        it('should detach the seesion from the event loop', function() {
            const profile = new Profile()
            const sess = new Session(profile)
            sess.close()
            sess.close()
            sess.close()
        })
    })
  describe('#context', () => {
        it('should return a context', () => {
            const profile = new Profile()
            const sess = new Session(profile)
            assert.equal(sess.context.constructor.name, 'Context')
            sess.close()
        })
    })
  describe('#pushFrames', () => {
        it('pushFrames', function() {
            const profile = new Profile()
            const sess = new Session(profile)
            for (i = 0; i < 100; ++i) {
                sess.pushFrames([{ data: ethData }], 1)
            }
            sess.close()
        })
    })
  describe('#frames', () => {
        it('should return an array', function() {
            const profile = new Profile()
            const sess = new Session(profile)
            assert(Array.isArray(sess.frames(0, 0)))
            sess.close()
        })
        it('should throw for wrong arguments', () => {
            const profile = new Profile()
            const sess = new Session(profile)
            assert.throws(() => sess.frames(), TypeError)
            assert.throws(() => sess.frames(0), TypeError)
            assert.throws(() => sess.frames(0, 'aaa'), TypeError)
            sess.close()
        })
    })
  describe('#filteredFrames', () => {
        it('should return an array', function() {
            const profile = new Profile()
            const sess = new Session(profile)
            assert(Array.isArray(sess.filteredFrames(0, 0, 0)))
            sess.close()
        })
        it('should throw for wrong arguments', () => {
            const profile = new Profile()
            const sess = new Session(profile)
            assert.throws(() => sess.filteredFrames(), TypeError)
            assert.throws(() => sess.filteredFrames(0), TypeError)
            assert.throws(() => sess.filteredFrames(0, 'aaa'), TypeError)
            assert.throws(() => sess.filteredFrames(0, 0, 'aaa'), TypeError)
            sess.close()
        })
    })

  /*
    Describe('#setFilter', function() {
        it('setFilter', function(done) {
            const profile = new Profile()
            const sess = new Session(profile)
            sess.on('event', (e) => {
                if (e.type === 'filtered_frames' && e.length === 50) {
                    sess.close()
                    done()
                }
            })
            sess.setFilter('main', 'aaaa')
            for (i = 0; i < 100; ++i) {
                sess.pushFrames([{ data: ethData }], 1)
            }
        })
    })
    */
})

const ethData = new Buffer([
  0xd4, 0xc3, 0xb2, 0xa1, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x33, 0xf6, 0x7e, 0x58, 0x88, 0x65,
  0x0d, 0x00, 0x42, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0xac, 0xbc, 0x32, 0xbc, 0x2a,
  0x87, 0x80, 0x13, 0x82, 0x62, 0xa2, 0x45, 0x08, 0x00, 0x45, 0x00, 0x00, 0x34, 0x69, 0xaf,
  0x40, 0x00, 0x31, 0x06, 0x01, 0xf7, 0xca, 0xe8, 0xee, 0x28, 0xc0, 0xa8, 0x64, 0x64, 0x00,
  0x50, 0xc4, 0x27, 0x22, 0xdd, 0xb1, 0xc0, 0x63, 0x6a, 0x47, 0x9b, 0x80, 0x10, 0x00, 0x72,
  0xf7, 0x6c, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0xf9, 0x28, 0x89, 0x4f, 0x61, 0x8f, 0x78,
  0x9d
])
