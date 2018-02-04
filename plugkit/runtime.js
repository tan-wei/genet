function value (val) {
  if (typeof val === 'object' && val !== null) {
    if (val.constructor.name === 'Attr') {
      return val.value
    }
  }
  return val
}

function resolver(root) {
  return function (id) {
    const attr = root.query(id)
    return attr === null ? undefined : attr
  }
}

function operator(opcode, lhs, rhs) {
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
        return false
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
        ? lhs
        : rhs
    default:
      return false
  }
}

(function (root) {
  return (function(__resolve, __operator, __value) {
    return @@@
  })(resolver(root), operator, value)
})
