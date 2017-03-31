const module = require('module')

let deplug = {

}

const load = module._load
module._load = (request, parent, isMain) => {
  if (request === 'deplug') {
    return deplug
  }
  return load(request, parent, isMain)
}
