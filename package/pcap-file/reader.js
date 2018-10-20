module.exports = (sess, arg) => {
  if (arg.file.endsWith('.pcap')) {
    sess.createReader('app.genet.reader.pcap-file', arg)
    return true
  }
}
