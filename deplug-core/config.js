import jsonfile from 'jsonfile'
import os from 'os'
import path from 'path'

const deplug = jsonfile.readFileSync(
  path.join(__dirname, '..', '..', 'package.json'))
const userPath = path.join(os.homedir(), '.deplug')
const userPluginPath = path.join(userPath, 'plugins')
const userProfilePath = path.join(userPath, 'profiles')
const builtinPluginPath = path.join(__dirname, '..', '..', 'plugins')
export default {
 deplug,
 userPath,
 userPluginPath,
 userProfilePath,
 builtinPluginPath,
}
