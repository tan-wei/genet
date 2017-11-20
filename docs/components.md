# Components

A package contains one or more components.

## Component Types

### Packet Dissector

```json
{
  "type": "core:dissector:packet",
  "main": "main.node"
}
```

### Stream Dissector

```json
{
  "type": "core:dissector:stream",
  "main": "main.node"
}
```

### Token

```json
{
  "type": "core:token",
  "main": "tokens.json"
}
```

### Attribute Renderer

```json
{
  "type": "core:renderer:attr",
  "id": "@ipv4:addr",
  "main": "main.js"
}
```

### Layer Renderer

```json
{
  "type": "core:renderer:layer",
  "id": "ipv4",
  "main": "main.js"
}
```

### Filter Transform

```json
{
  "type": "core:filter:transform",
  "main": "main.js"
}
```
