import os from 'os'
import path from 'path'
import { readJsonSync } from 'fs-extra'

function readFile(filePath: string) {
  try {
    return readJsonSync(filePath)
  } catch (err) {
    return {}
  }
}

function getRootPath() {
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
const genet = readJsonSync(path.join(rootPath, 'package.json'))
const userPath = path.join(os.homedir(), '.genet')
const userPackagePath = path.join(userPath, 'package')
const userProfilePath = path.join(userPath, 'profile')
const builtinPackagePath = path.join(rootPath, 'package')
const cachePath = path.join(os.homedir(), '.genet-cache')
export default class Env {
  static readonly genet = genet
  static readonly rootPath = rootPath
  static readonly userPath = userPath
  static readonly userPackagePath = userPackagePath
  static readonly userProfilePath = userProfilePath
  static readonly builtinPackagePath = builtinPackagePath
  static readonly cachePath = cachePath
  static readonly linuxIconPath = '/usr/share/icons/hicolor/256x256/apps/genet.png'
}
