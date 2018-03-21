
/* eslint prefer-reflect: "off"*/
/* eslint consistent-this: "off" */
/* eslint no-invalid-this: "off" */

const acorn = require('acorn')
const macroToken = new acorn.TokenType('@')
class Parser {
  constructor (inst) {
    this.override(inst, this.getTokenFromCode)
    this.override(inst, this.parseExprAtom)
    this.override(inst, this.parseExpressionStatement)
    this.override(inst, this.parseIdent)
  }

  override (inst, func) {
    inst.extend(func.name, (inner) => {
      const self = this
      return function rrr (...args) {
        const super_ = (...supArgs) => inner.call(this, ...supArgs)
        super_.inst = this
        return func.call(self, super_, ...args)
      }
    })
  }

  getTokenFromCode (super_, code) {
    if (code === 64) {
      const [macro, value] = (/@(\S*)/).exec(
        super_.inst.input.substr(super_.inst.pos)
      )
      super_.inst.pos += macro.length
      return super_.inst.finishToken(macroToken, value)
    }
    return super_(code)
  }

  parseExprAtom (super_, refDestructuringErrors) {
    if (super_.inst.type === macroToken) {
      const node = super_.inst.startNode()
      node.name = super_.inst.value
      node.extended = 'macro'
      super_.inst.next()
      return super_.inst.finishNode(node, 'Identifier')
    }
    return super_(refDestructuringErrors)
  }

  parseExpressionStatement (super_, node, expr) {
    if (super_.inst.type === acorn.tokTypes.name) {
      return this.parsePipeline(super_, node, expr)
    }
    return super_(node, expr)
  }

  parseIdent (super_, liberal, isBinding) {
    if (super_.inst.type === acorn.tokTypes.name &&
         (/^[a-z_$]\w*$/).test(super_.inst.value)) {
      let name = super_.inst.value
      super_.inst.next()
      for (;;) {
        if (!super_.inst.eat(acorn.tokTypes.dot)) {
          break
        }
        name += `.${super_.inst.value}`
        super_.inst.expect(acorn.tokTypes.name)
      }
      const node = super_.inst.startNode()
      node.name = name
      node.extended = 'attr'
      return super_.inst.finishNode(node, 'Identifier')
    }
    return super_(liberal, isBinding)
  }

  parsePipeline (super_, node, expr) {
    const funcs = []
    for (;;) {
      const funcname = super_.inst.value
      if (!super_.inst.eat(acorn.tokTypes.name)) {
        break
      }
      funcs.push({
        name: funcname,
        args: [],
      })
      if (super_.inst.type !== acorn.tokTypes.name) {
        for (;;) {
          if (!super_.inst.eat(acorn.tokTypes.colon)) {
            break
          }
          const arg = super_.inst.parseExpression()
          funcs[funcs.length - 1].args.push(arg)
        }
      }
    }
    if (funcs.length > 0) {
      node.extended = 'pipeline'
      node.pipeline = funcs
    }
    return super_(node, expr)
  }
}

module.exports = function create (inst) {
  return new Parser(inst)
}
