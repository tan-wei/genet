Variant var;
Variant *item1 = Variant_mapValueRef(&var, "key", -1);
Variant_setUint32(item1, 5);
Variant *item2 = Variant_mapValueRef(&var, "hello", -1);
Variant_setString(item2, "world", -1);
// var is equivalent to {key: 5, hello: "world"} in JavaScript
