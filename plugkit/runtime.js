function value (val) {
  if (typeof val === 'object' && val !== null) {
    if (val.constructor.name === 'Attr') {
      return val.value
    }
  }
  return val
}

function resolver(root) {
  return function (id, ...args) {
    if (typeof id === 'string' && id.startsWith('_')) {
      id = root.primaryLayer.id + id.slice(1)
    }
    const attr = root.query(id)
    if (attr === null) return undefined
    let result = attr
    for (const id of args) {
      if (typeof result !== 'object') return undefined
      if (id in result) {
        if (typeof result[id] === 'function') {
          result = result[id].bind(result)
        } else {
          result = result[id]
        }
        continue
      }
      const val = value(result)
      if (typeof val !== 'object') return undefined
      if (id in val) {
        if (typeof val[id] === 'function') {
          result = val[id].bind(val)
        } else {
          result = val[id]
        }
        continue
      }
      return undefined
    }
    return result
  }
}

function operator(opcode, lhs, ...args) {
  if (opcode === '()') {
    if (typeof lhs === 'function') {
      return lhs(...args)
    }
    return undefined
  }

  const rhs = args[0]
  if (arguments.length === 2) {
    switch (opcode) {
      case '+':
        return Number(value(lhs))
      case '-':
        return -value(lhs)
      case '~':
        return ~value(lhs)
      case '!':
        return !value(lhs)
      case 'typeof':
        return typeof lhs
      default:
        return false
    }
  }

  const left = value(lhs)
  const right = value(rhs)
  if (typeof left === 'object' && left !== null && Symbol.iterator in left &&
    typeof right === 'object' && right !== null && Symbol.iterator in right) {
    const leftIt = left[Symbol.iterator]()
    const rightIt = right[Symbol.iterator]()
    switch (opcode) {
      case '==':
      case '===':
        for (;;) {
          const leftValue = leftIt.next()
          const rightValue = rightIt.next()
          if (leftValue.value !== rightValue.value ||
            leftValue.done !== rightValue.done) return false
          if (leftValue.done && rightValue.done) return true
        }
      case '!=':
      case '!==':
        for (;;) {
          const leftValue = leftIt.next()
          const rightValue = rightIt.next()
          if (leftValue.value !== rightValue.value ||
            leftValue.done !== rightValue.done) return true
          if (leftValue.done && rightValue.done) return false
        }
      case '<':
        for (;;) {
          const leftValue = leftIt.next()
          const rightValue = rightIt.next()
          if (leftValue.value < rightValue.value) return true
          if (leftValue.value > rightValue.value) return false
          if (leftValue.done || rightValue.done) return false
        }
      case '>':
        for (;;) {
          const leftValue = leftIt.next()
          const rightValue = rightIt.next()
          if (leftValue.value > rightValue.value) return true
          if (leftValue.value < rightValue.value) return false
          if (leftValue.done || rightValue.done) return false
        }
      case '>=':
        for (;;) {
          const leftValue = leftIt.next()
          const rightValue = rightIt.next()
          if (leftValue.value > rightValue.value) return true
          if (leftValue.value < rightValue.value) return false
          if (leftValue.done || rightValue.done) return true
        }
      default:
        ;
    }
  }

  switch (opcode) {
    case 'instanceof':
      return left instanceof right
    case 'in':
      return left in right
    case '+':
      return left + right
    case '-':
      return left - right
    case '*':
      return left * right
    case '/':
      return left / right
    case '%':
      return left % right
    case '**':
      return left ** right
    case '|':
      return left | right
    case '^':
      return left ^ right
    case '&':
      return left & right
    case '==':
      return left == right
    case '!=':
      return left != right
    case '===':
      return left === right
    case '!==':
      return left !== right
    case '<':
      return left < right
    case '>':
      return left > right
    case '<=':
      return left <= right
    case '<<':
      return left << right
    case '>>':
      return left >> right
    case '>>>':
      return left >>> right
    case '||':
      return left
        ? lhs
        : rhs
    case '&&':
      return left
        ? rhs
        : lhs
    default:
      return false
  }
}

@@globals@@

(function (root) {
  return (function(__resolve, __operator, __value) {
    return @@expression@@
  })(resolver(root), operator, value)
})
