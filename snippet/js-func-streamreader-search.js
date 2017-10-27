const reader = new StreamReader()
reader.addSlice(slice)

const offset = reader.search('\r\n')
if (offset >= 0) {
  // Found
}
