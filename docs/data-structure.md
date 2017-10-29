
# Data Structure

## Frame

Frame represents a single packet.

Each frame has a root layer.
This is, in most cases, a link layer protocol such as Ethernet.

## Layer

Layer represents a layer in a protocol stack.

### Layer Tree

```
 - eth - ipv4 - udp - rudp  <- Leaf Layer (Primary)
                    ` utp   <- Leaf Layer
```

!> In a dissection, you can manipulate the parent layer but the grandparent layer
and its ancestors are read-only because they could be accessed by multiple threads.

### Worker ID

The dissection processes in Deplug are concurrent.
In other words, packets will be dissected out of order.

Worker ID is important when you want to process a series of layers in order.
Layers which have the same worker ID are always processed in the same thread.

### Confidence

_Confidence_ is a 2-bit value represents an accuracy of a dissected layer.

|Level    |Value|
|---------|-----|
|Error    |0    |
|Possible |1    |
|Probable |2    |
|Exactly  |3    |

## Attr

## Payload


## Token

Token is a constant integer value associated with a string, such as Symbol in JavaScript.

To get the token for "string", use [Token.get("string")](diss-api-js.md#js-func-token-get).
It returns a 32-bit unsigned integer.　
([Token_get](diss-api-c.md#c-func-token-get) is available for C-API)

Some strings are predefined though you can get tokens for any strings. When the token for an unregisterd string is requested, the string is added to the global lookup table with an unique integer.

There is [Token.string](diss-api-js.md#js-func-token-string)
(and [Token_string](diss-api-c.md#c-func-token-string) for C-API) for the reverse lookup.

!> `Token.get` may return different value in another process.
