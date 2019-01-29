module.exports = (sess, url) => {
  if (url.toString().endsWith('.genet')) {
    sess.createReader('app.genet.reader.genet-file', url)
    return true
  }
}
