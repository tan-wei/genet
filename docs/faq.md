# FAQ

## Analyzer

### Packets are detected as a wrong protocols

Deplug's approach to the packet analysis is *"Trying all possible interpretation"*.
So if the analyzer does not know a correct way to interpret the packets, it returns the most likely results instead.

You can discard uncertain results by changing the `Confidence Threshold` level.

## Package

### Changing a package configuration does not take effect

Some packages (especially native modules) have to be configured before the initialization.

It is recommended to reopen tabs or restart the app to apply changes correctly.
