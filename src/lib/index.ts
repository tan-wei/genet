import Genet from './genet'

let root: any = null
const handler = {
  get(target, prop) {
    if (prop === '__esModule') {
      return null
    }
    if (root === null) {
      if (prop === 'init') {
        return (obj) => {
          root = obj
        }
      }
      throw new Error('not initialized')
    }
    return root[prop]
  },
}

const proxy: Genet = new Proxy({}, handler)

// @ts-ignore
export = proxy
