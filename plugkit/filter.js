function $_value(value) {
  if (typeof value === 'object' && value !== null) {
    if (value.constructor.name === 'Attr') {
      return value.value
    }
  }
  return value
}

function $_op(opcode, ...args) {
  if (args.length === 1) {
    switch (opcode) {
    case '+':
      return +$_value(args[0])
    case '-':
      return -$_value(args[0])
    case '~':
      return ~$_value(args[0])
    case '!':
      return !$_value(args[0])
    case 'delete':
      return delete args[0]
    case 'void':
      return void args[0]
    case 'typeof':
      return typeof args[0]
    }
  } else if (args.length === 2) {
    const left = $_value(args[0])
    const right = $_value(args[1])

    if (typeof left === 'object' && Symbol.iterator in left
        && typeof right === 'object' && Symbol.iterator in right) {
      const leftIt = left[Symbol.iterator]()
      const rightIt = right[Symbol.iterator]()
      switch (opcode) {
        case '==':
        case '===':
        {
          while (true) {
            const leftValue = leftIt.next()
            const rightValue = rightIt.next()
            if (leftValue.value !== rightValue.value ||
              leftValue.done !== rightValue.done) return false
            if (leftValue.done && rightValue.done) return true
          }
        }
        break
        case '!=':
        case '!==':
        {
          while (true) {
            const leftValue = leftIt.next()
            const rightValue = rightIt.next()
            if (leftValue.value !== rightValue.value ||
              leftValue.done !== rightValue.done) return true
            if (leftValue.done && rightValue.done) return false
          }
        }
        break
        case '<':
        {
          while (true) {
            const leftValue = leftIt.next()
            const rightValue = rightIt.next()
            if (leftValue.value < rightValue.value) return true
            if (leftValue.value > rightValue.value) return false
            if (leftValue.done || rightValue.done) return false
          }
        }
        break
        case '<=':
        {
          while (true) {
            const leftValue = leftIt.next()
            const rightValue = rightIt.next()
            if (leftValue.value < rightValue.value) return true
            if (leftValue.value > rightValue.value) return false
            if (leftValue.done || rightValue.done) return true
          }
        }
        break
        case '>':
        {
          while (true) {
            const leftValue = leftIt.next()
            const rightValue = rightIt.next()
            if (leftValue.value > rightValue.value) return true
            if (leftValue.value < rightValue.value) return false
            if (leftValue.done || rightValue.done) return false
          }
        }
        break
        case '>=':
        {
          while (true) {
            const leftValue = leftIt.next()
            const rightValue = rightIt.next()
            if (leftValue.value > rightValue.value) return true
            if (leftValue.value < rightValue.value) return false
            if (leftValue.done || rightValue.done) return true
          }
        }
        break
      }
    }

    switch (opcode) {
    case 'instanceof':
      return args[0] instanceof args[1]
    case 'in':
      return args[0] in args[1]
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
      return !!left ? args[0] : args[1]
    case '&&':
      return !left ? args[0] : args[1]
    }
  }
  return false
}
