const reader = new StreamReader()
reader.addSlice(...)

const offset = reader.search('\r\n')
if (offset >= 0) {
  // found
}
