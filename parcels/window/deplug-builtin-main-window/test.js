import jquery from 'jquery'
export default function() {
  console.log('aaa')
  jquery('head')
    .append(jquery(`<link rel="import" href="${__dirname}/elements.htm">`))
}
