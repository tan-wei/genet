
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
|Decodable|0    |
|Possible |1    |
|Probable |2    |
|Exactly  |3    |

## Attr

## Payload
