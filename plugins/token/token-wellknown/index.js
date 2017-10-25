import jsonfile from 'jsonfile'
import path from 'path'

export default {
  components: [
    {
      _type: 'core:attribute',
      attributes: jsonfile.readFileSync(path.join(__dirname, 'tokens.json')),
    }
  ],
}
