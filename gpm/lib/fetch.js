const protocols = [
  require('./protocol/local'),
  require('./protocol/git')
]
async function fetchPackage (url) {
  const availables = await
    Promise.all(protocols.map((proto) => proto.isSupported(url)))

  const proto = protocols.find((item, index) => availables[index])
  if (!proto) {
    throw new Error('unsupported protocol')
  }

  return proto.fetch(url)
}

module.exports = fetchPackage
