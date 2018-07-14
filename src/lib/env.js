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
    if (readFile(pkg).name === 'genet') {
      return root
    }
    root = path.dirname(root)
  }
  return root
}

const rootPath = getRootPath()
const genet = jsonfile.readFileSync(path.join(rootPath, 'package.json'))
const userPath = path.join(os.homedir(), '.genet')
const userPackagePath = path.join(userPath, 'package')
const userProfilePath = path.join(userPath, 'profile')
const builtinPackagePath = path.join(rootPath, 'package')
const cachePath = path.join(os.homedir(), '.genet-cache')
export default {
  genet,
  rootPath,
  userPath,
  userPackagePath,
  userProfilePath,
  builtinPackagePath,
  cachePath,
  linuxIconPath: '/usr/share/icons/hicolor/256x256/apps/deplug.png',
}
