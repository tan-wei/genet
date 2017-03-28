module.exports = (sess, arg) => {
    if (arg.file.endsWith('.pcap')) {
        sess.createReader('pcap-file', arg)
        return true
    }
}
