# FAQ

## Analyzer

### Packets are detected as a wrong protocol

Deplug's approach to the packet analysis is *"Trying all possible interpretation"*.
So if the analyzer does not know a correct way to interpret the packets, it returns the most likely results instead.

You can discard uncertain results by changing the `Confidence Threshold` level.

## Package

### Changing a package configuration does not take effect

Some packages (especially native modules) have to be configured before the initialization.

It is recommended to reopen tabs or restart the app to apply changes correctly.

## Filter

### How to filter frames by an attribute which has no value.

```
tcp.options.nop           # evaluated as null (falsy value)
tcp.options.nop == null   # cannot distinguish null and undefined
tcp.options.nop === null  # correct
```

## Debug

### How to debug native modules?

1. `View` -> `Open Developer Tools for Current Tab`.
2. Type `process.pid` in the console to get the process ID.
3. Use `Attach to Process` feature in your favorite debugger.
   - [gdb](ftp://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_22.html)
   - [Xcode](https://developer.apple.com/library/content/documentation/DeveloperTools/Conceptual/debugging_with_xcode/chapters/debugging_tools.html)
   - [Visual Studio](https://msdn.microsoft.com/en-us/library/3s68z0b3.aspx)
