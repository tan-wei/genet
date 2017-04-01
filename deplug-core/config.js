import os from 'os'
import path from 'path'

const Package = require(path.join(__dirname, '..', '..', 'package.json'))
const UserPath = path.join(os.homedir(), '.deplug')
const UserPackagePath = path.join(UserPath, 'packages')
const UserProfilePath = path.join(UserPath, 'profiles')
const BuiltinPackagePath = path.join(__dirname, '..', 'packages')
export default {
 Package,
 UserPath,
 UserPackagePath,
 UserProfilePath,
 BuiltinPackagePath,
}
