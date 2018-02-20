# deplug

[![Build Status](https://travis-ci.org/deplug/deplug.svg)](https://travis-ci.org/deplug/deplug)
[![Build status](https://ci.appveyor.com/api/projects/status/37ek4o97h0kqr0p9?svg=true)](https://ci.appveyor.com/project/h2so5/deplug)
[![Current Version](https://img.shields.io/crates/v/plugkit.svg)](https://crates.io/crates/plugkit)

# Download

## Latest Releases

(via CDN)

- macOS
  - [![](https://cdn.deplug.net/deplug/release/latest/deplug-darwin-amd64.dmg.ver.svg) Download .dmg](https://cdn.deplug.net/deplug/release/latest/deplug-darwin-amd64.dmg)
- Linux
  - [![](https://cdn.deplug.net/deplug/release/latest/deplug-linux-amd64.deb.ver.svg) Download .deb](https://cdn.deplug.net/deplug/release/latest/deplug-linux-amd64.deb)
  - [![](https://cdn.deplug.net/deplug/release/latest/deplug-linux-amd64.rpm.ver.svg) Download .rpm](https://cdn.deplug.net/deplug/release/latest/deplug-linux-amd64.rpm)
- Windows
  - [![](https://cdn.deplug.net/deplug/release/latest/deplug-win-amd64.exe.ver.svg) Download .exe (Installer)](https://cdn.deplug.net/deplug/release/latest/deplug-win-amd64.exe)

## Nightly Builds

- macOS
  - [![](https://cdn.deplug.net/deplug/nightly/deplug-darwin-amd64.dmg.ver.svg) Download .dmg](https://cdn.deplug.net/deplug/nightly/deplug-darwin-amd64.dmg)
- Linux
  - [![](https://cdn.deplug.net/deplug/nightly/deplug-linux-amd64.deb.ver.svg) Download .deb](https://cdn.deplug.net/deplug/nightly/deplug-linux-amd64.deb)
  - [![](https://cdn.deplug.net/deplug/nightly/deplug-linux-amd64.rpm.ver.svg) Download .rpm](https://cdn.deplug.net/deplug/nightly/deplug-linux-amd64.rpm)
- Windows
  - [![](https://cdn.deplug.net/deplug/nightly/deplug-win-amd64.exe.ver.svg) Download .exe (Installer)](https://cdn.deplug.net/deplug/nightly/deplug-win-amd64.exe) 

## Older Releases

- https://github.com/deplug/release-build/releases

## Build from Source

See https://docs.deplug.net/build-from-source.html

# FAQ

## Why does Deplug use Electron as a GUI framework?

### Web standards
Deplug GUI is almost made of standardized HTML/CSS/JavaScript in contrast to 
traditional cross-platform GUI toolkits that have the enclosed ecosystem, such as Qt, GTK+.

### Scripting support
Needless to say, GUI is fully accessible by scripts.  
In addition, JavaScript is also used in the display filter and the analyzer.
Those JavaScript environments can be shared with the Chrome's V8 runtime.

### Packet analyzer does not need a performance-critical GUI
One of the big downside of using electron is the performance issue due to the heavy DOM manipulation.
However, this is not a serious issue in this project, compared to writing a text editor etc.  
(This does not mean unconcern about the lightweight GUI, indeed)
