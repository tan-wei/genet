import estraverse from 'estraverse'

export default function(ast)
{
  return estraverse.replace(ast, {
    enter: (node) => {
      if (node.type === 'TemplateLiteral')
        return {type: 'Literal', value: '"ssss"'}
    }
  })
}
