module.exports = (sess, arg) => {
  if (arg.file.endsWith('.genet')) {
    sess.createReader('app.genet.reader.genet-file', arg)
    return true
  }
}
