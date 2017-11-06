import jsonfile from 'jsonfile'
import os from 'os'
import path from 'path'

function readFile (filePath) {
    try {
        return jsonfile.readFileSync(filePath)
    } catch (err) {
        return {}
    }
}

function getRootPath () {
    let root = __dirname
    while (root !== '/') {
        const pkg = path.join(root, 'package.json')
        if (readFile(pkg).name === 'deplug') {
            return root
        }
        root = path.dirname(root)
    }
    return root
}

const root = getRootPath()
const deplug = jsonfile.readFileSync(path.join(root, 'package.json'))
const userPath = path.join(os.homedir(), '.deplug')
const userPackagePath = path.join(userPath, 'package')
const userProfilePath = path.join(userPath, 'profile')
const builtinPackagePath = path.join(root, 'package')
export default {
  deplug,
  userPath,
  userPackagePath,
  userProfilePath,
  builtinPackagePath,
  linuxIconPath: '/usr/share/icons/hicolor/256x256/apps/deplug.png',
}
