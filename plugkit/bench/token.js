const { Token } = require('..')
const { Suite } = require('benchmark')

const ethToken = Token.get('eth')
const longToken = Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320')
const suite = new Suite()
suite
.add('Token#get []', () => {
  Token.get()
})
.add('Token#string []', () => {
  Token.string(0)
})
.add('Token#get [eth]', () => {
  Token.get('eth')
})
.add('Token#string [eth]', () => {
  Token.string(ethToken)
})
.add('Token#get [9bbb35e6...]', () => {
  Token.get('9bbb35e6-cde1-419b-8d7b-ea04c87e7320')
})
.add('Token#string [9bbb35e6...]', () => {
  Token.string(longToken)
})
.add('Token#join [eth + 9bbb35e6...]', () => {
  Token.join(ethToken, '9bbb35e6-cde1-419b-8d7b-ea04c87e7320')
})
.on('cycle', (event) => {
  // eslint-disable-next-line no-console
  console.log(String(event.target))
})
.run({ 'async': true })
