import { EventEmitter } from 'events'
import deepEqual from 'deep-equal'
import env from './env'
import fetch from 'node-fetch'
import objpath from 'object-path'
import semver from 'semver'
import yaml from 'js-yaml'

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
  if (entry.source === 'npm') {
    const encodedName = entry.name.replace('/', '%2F')
    const url = `https://registry.npmjs.org/${encodedName}`
    const meta = await fetch(url)
      .then((res) => res.text())
      .then((data) => JSON.parse(data))

    const vers = Object.keys(meta.versions)
    vers.sort(semver.rcompare)

    const pkg = meta.versions[vers[0]]
    const engineVersion = objpath.get(pkg, 'engines.deplug', null)
    if (engineVersion === null) {
      throw new Error('Incompatible package')
    }
    if (!semver.satisfies(env.deplug.version, engineVersion)) {
      throw new Error('Deplug version mismatch')
    }
    const tarball = objpath.get(pkg, 'dist.tarball', null)
    if (tarball === null) {
      throw new Error('Tarball not found')
    }

    return {
      data: pkg,
      timestamp: new Date(meta.time.modified),
      archive: tarball,
      url: entry.url || null,
    }
  }
  throw new Error('Unsupported source type')
}

async function crawlRegistries (registries, errorCb) {
  const tasks = []
  for (const reg of registries) {
    tasks.push(
      fetch(reg)
        .then((res) => res.text())
        .then((data) => Object.values(yaml.safeLoad(data)))
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
    const registries = config.get('_.packageRegistries', [])
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
