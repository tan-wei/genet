# Build from Source

## Requirements

### Common

- git
- Node.js 8.x

### macOS

- Xcode 9

### Linux

- gcc
- libpcap-dev
- libcap-dev
- gperf

```bash
# Ubuntu
$ sudo apt-get install build-essential libpcap-dev libcap-dev gperf
```

### Windows

- Visual Studio 2015/2017

## Build

```bash
$ git clone https://github.com/deplug/deplug.git
$ cd deplug
$ npm install
$ npm run watch
```

## Build Documentation

(macOS only)

```bash
$ make docs-serve
# open http://localhost:3000/
```
