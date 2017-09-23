const kit = require('bindings')('plugkit_test.node')
const result = kit.Testing.runCApiTests()
process.exit(result)
