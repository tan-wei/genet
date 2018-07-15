import { EventEmitter } from 'events'
import axios from 'axios'
import deepEqual from 'deep-equal'
import env from './env'
import normalize from 'normalize-url'
import objpath from 'object-path'
import semver from 'semver'

axios.defaults.adapter = require('axios/lib/adapters/http')

function promiseFilter (proms, errorCb) {
  if (proms.length === 0) {
    return Promise.resolve([])
  }
  return new Promise((res) => {
    let count = proms.length
    const filtered = []
    function update (result) {
      if (arguments.length > 0) {
        filtered.push(result)
      }
      count -= 1
      if (count === 0) {
        res(filtered)
      }
    }

    for (const prom of proms) {
      prom.then((result) => {
        update(result)
      }).catch((err) => {
        update()
        if (errorCb) {
          errorCb(err)
        }
      })
    }
  })
}

async function resolveEntry (entry) {
  const normalized = normalize(entry, { stripWWW: false })
  const prefix = 'https://www.npmjs.com/package/'
  if (normalized.startsWith(prefix)) {
    const name = normalized.slice(prefix.length)
    const encodedName = name.replace('/', '%2F')
    const url = `https://registry.npmjs.com/${encodedName}`
    const meta = await axios.get(url)
      .then((res) => res.data)

    const vers = Object.keys(meta.versions)
    vers.sort(semver.rcompare)

    const pkg = meta.versions[vers[0]]
    const engineVersion = objpath.get(pkg, 'engines.genet', null)
    if (engineVersion === null) {
      throw new Error('Incompatible package')
    }
    if (!semver.satisfies(semver.coerce(env.genet.version), engineVersion)) {
      throw new Error('genet version mismatch')
    }
    const tarball = objpath.get(pkg, 'dist.tarball', null)
    if (tarball === null) {
      throw new Error('Tarball not found')
    }

    return {
      id: `npmjs.com/${name}`,
      data: pkg,
      timestamp: new Date(meta.time.modified),
      archive: tarball,
    }
  }
  throw new Error('Unsupported source type')
}

async function crawlRegistries (registries, errorCb) {
  const tasks = []
  for (const reg of registries) {
    tasks.push(
      axios(reg)
        .then((res) => res.data)
        .then((data) => data.split('\n').filter((url) => url.trim()))
        .then((entries) => promiseFilter(entries.map(resolveEntry), errorCb))
    )
  }
  const data = await promiseFilter(tasks, errorCb)
  return data.reduce((lhs, rhs) => lhs.concat(rhs), [])
}

const fields = Symbol('fields')
export default class PackageRegistry extends EventEmitter {
  constructor (profile, config, cache) {
    super()
    const registries = config.get('_.package.registries', [])
    this[fields] = {
      registries,
      updating: false,
      packages: [],
      lastUpdated: null,
      cache,
    }
  }

  async update () {
    const { registries, cache } = this[fields]
    if (this[fields].updating || registries.length === 0) {
      return
    }
    this[fields].lastUpdated = new Date()
    this[fields].updating = true
    const cached = cache.get('core:package:registry:cache')
    if (typeof cached !== 'undefined' &&
      !deepEqual(cached, this[fields].packages)) {
      this[fields].packages = cached
      this.emit('updated')
    }
    const packages = await crawlRegistries(registries, (err) => {
      this.emit('error', err)
    })
    this[fields].packages = packages
    this[fields].updating = false
    cache.set('core:package:registry:cache', packages)
    this.emit('updated')
  }

  get packages () {
    return this[fields].packages
  }

  get lastUpdated () {
    return this[fields].lastUpdated
  }
}
