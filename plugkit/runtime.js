function resolver(root) {
  return function (id) {
    const attr = root.query(id)
    if (attr === null) return undefined
    if ('value' in attr) {
      return attr.value
    }
    return attr
  }
}

function pipeline(name, self, ...args) {
  const val = Object(self)
  if (name in val && typeof val[name] === 'function') {
    return val[name](...args)
  }
  return val
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
        return +lhs
      case '-':
        return -lhs
      case '~':
        return ~lhs
      case '!':
        return !lhs
      case 'typeof':
        return typeof lhs
      default:
        return false
    }
  }

  const left = lhs
  const right = rhs
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
      case '<=':
        for (;;) {
          const leftValue = leftIt.next()
          const rightValue = rightIt.next()
          if (leftValue.value < rightValue.value) return true
          if (leftValue.value > rightValue.value) return false
          if (leftValue.done || rightValue.done) return true
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
    case '>=':
      return left >= right
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

(function (root) {
  return (function(__resolve, __operator, __pipeline) {
    return @@expression@@
  })(resolver(root), operator, pipeline)
})
