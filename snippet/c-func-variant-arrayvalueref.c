Variant var;
for (int i = 0; i < 5; ++i) {
  Variant *item = Variant_arrayValueRef(&var, i);
  Variant_setUint32(item, i + 1);
}
// var is equivalent to [1, 2, 3, 4, 5] in JavaScript
