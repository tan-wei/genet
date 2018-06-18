const { Session } = require('..')
const { Profile } = Session

const assert = require('assert')
describe('Context', function() {
    describe('#token', function() {
        it('should throw for wrong arguments', () => {
          const profile = new Profile()
          const sess = new Session(profile)
          const ctx = sess.context
          assert.throws(() => ctx.token(), TypeError)
          assert.throws(() => ctx.token(0), TypeError)
          sess.close()
        })
        it('should return a token', function() {
          const profile = new Profile()
          const sess = new Session(profile)
          const ctx = sess.context
          assert.equal(ctx.token('a'), ctx.token('a'))
          assert.equal(ctx.token('b'), ctx.token('b'))
          assert.notEqual(ctx.token('a'), ctx.token('b'))
          sess.close()
        })
      })
      describe('#string', function() {
        it('should throw for wrong arguments', () => {
          const profile = new Profile()
          const sess = new Session(profile)
          const ctx = sess.context
          assert.throws(() => ctx.string(), TypeError)
          assert.throws(() => ctx.string('aaa'), TypeError)
          sess.close()
        })
        it('should return a string', function() {
          const profile = new Profile()
          const sess = new Session(profile)
          const ctx = sess.context
          assert.equal(ctx.string(ctx.token('a')), 'a')
          assert.equal(ctx.string(ctx.token('bb')), 'bb')
          assert.equal(ctx.string(ctx.token('ccc')), 'ccc')
          sess.close()
        })
    })
    describe('#getConfig', function() {
      it('should throw for wrong arguments', () => {
        const profile = new Profile()
        const sess = new Session(profile)
        const ctx = sess.context
        assert.throws(() => ctx.getConfig(), TypeError)
        assert.throws(() => ctx.getConfig(0), TypeError)
        sess.close()
      })
      it('should return a config value', () => {
        const profile = new Profile()
        profile.setConfig('aaa', {this_is: 'test'})
        const sess = new Session(profile)
        const ctx = sess.context
        assert.deepEqual(ctx.getConfig('aaa'), {this_is: 'test'})
        sess.close()
      })
    })
})