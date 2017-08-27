export default function(value)
{
  if(typeof value !== 'string') return null;
  const array = JSON.stringify(value.split('.').map(octet => parseInt(octet)))
  return `Uint8Array.from(${array})`
}
