import jsonfile from 'jsonfile'
import path from 'path'

export default {
  components: [
    {
      type: 'core:attribute',
      attribute: jsonfile.readFileSync(path.join(__dirname, 'tokens.json'))
    }
  ]
}
