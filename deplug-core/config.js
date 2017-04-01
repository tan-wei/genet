import jsonfile from 'jsonfile'
import os from 'os'
import path from 'path'

const deplug = jsonfile.readFileSync(
  path.join(__dirname, '..', '..', 'package.json'))
const userPath = path.join(os.homedir(), '.deplug')
const userParcelPath = path.join(userPath, 'parcels')
const userProfilePath = path.join(userPath, 'profiles')
const builtinParcelPath = path.join(__dirname, '..', '..', 'parcels')
export default {
 deplug,
 userPath,
 userParcelPath,
 userProfilePath,
 builtinParcelPath,
}
