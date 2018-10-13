const inquirer = require('inquirer')
const program = require('commander')
const semver = require('semver')
const { intersect } = require('semver-intersect')
const fs = require('fs-extra')
const path = require('path')
const TOMLStream = require('toml-stream')
const { version, genetAbiVersion } = require('./lib/env')

const templates = [
  {
    value: 'Panel',
    func: require('./lib/template/panel'),
  },
  {
    value: 'Decoder',
    func: require('./lib/template/decoder'),
  },
  {
    value: 'Reader',
    func: require('./lib/template/reader'),
  },
  {
    value: 'Writer',
    func: require('./lib/template/writer'),
  }
]
program
  .version(version)
  .usage('[name]')
  .option('-f, --force', 'overwrite non-empty directory')
  .parse(process.argv)

if (program.args.length > 1) {
  program.help()
}

const targetDir = path.resolve(program.args[0] || '.')
fs.ensureDirSync(targetDir)
if (fs.readdirSync(targetDir).length > 0 && !program.force) {
  console.warn(`[Error] Directroy is not empty: ${targetDir}`)
  console.warn('To ignore this error, use --force flag.')
  process.exit(1)
}

const sdkVersion = {
  type: 'input',
  name: 'genet-sdk Version',
  validate: (val) => {
    try {
      intersect(val, '>=0')
      return true
    } catch (err) {
      return false
    }
  },
}
if (genetAbiVersion) {
  sdkVersion.default = genetAbiVersion
}

inquirer
  .prompt([
    {
      type: 'input',
      name: 'Name',
      default: path.basename(path.resolve(targetDir)),
    },
    {
      type: 'input',
      name: 'Description',
    },
    {
      type: 'input',
      name: 'Version',
      default: '1.0.0',
      validate: (val) => semver.valid(val) !== null,
    },
    sdkVersion,
    {
      type: 'input',
      name: 'Author',
    },
    {
      type: 'input',
      name: 'License',
      default: 'MIT',
    },
    {
      type: 'checkbox',
      name: 'Templates',
      message: 'Choose templates what you want',
      choices: templates,
    }
  ])
  .then((answers) => {
    const components = []
    const files = {}
    for (const id of answers.Templates) {
      const tmp = templates.find(({ value }) => value === id)
      if (tmp) {
        tmp.func(answers, components, files)
      }
    }
    files['package.json'] = {
      name: answers.Name,
      description: answers.Description,
      author: answers.Author,
      version: answers.Version,
      engines: { genet: '*' },
      genet: { components },
    }
    for (const [name, data] of Object.entries(files)) {
      const file = path.join(targetDir, name)
      switch (path.extname(name)) {
        case '.json':
          fs.outputJsonSync(file, data, {
            spaces: '  ',
            replacer: (key, value) => {
              if (value !== '') {
                return value
              }
            },
          })
          break
        case '.toml':
          {
            fs.ensureFileSync(file)
            const stream = new TOMLStream()
            stream.pipe(fs.createWriteStream(file))
            stream.end(data)
          }
          break
        default:
          fs.outputFileSync(file, `${data}`)
          break
      }
    }
    console.log(`🎁 Package files are created in ${targetDir} !`)
  })
