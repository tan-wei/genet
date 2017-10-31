import jsonfile from 'jsonfile'
import os from 'os'
import path from 'path'

const deplug = jsonfile.readFileSync(
  path.join(__dirname, '..', '..', '..', 'package.json'))
const userPath = path.join(os.homedir(), '.deplug')
const userPluginPath = path.join(userPath, 'plugin')
const userProfilePath = path.join(userPath, 'profile')
const builtinPluginPath = path.join(__dirname, '..', '..', 'plugin')
export default {
 deplug,
 userPath,
 userPluginPath,
 userProfilePath,
 builtinPluginPath,
 linuxIconPath: '/usr/share/icons/hicolor/256x256/apps/deplug.png',
}
