# Install

## dpm

`dpm`(Deplug Package Manager) is a tool to manage packages from the command-line.

!> You have to launch Deplug app once before using `dpm`
or it cannot determine the version of Deplug.

### Installation

```bash
$ npm install -g @deplug/dpm
```

### Commands

#### dpm update [dirs...]

Update installed packages.

Check the timestamp of the installed package directories and run `npm install` if updated.
This is convenient especially for developing native packages.

## Build Native Modules

You need `node-gyp` to install packages contain native modules.

Visit https://github.com/nodejs/node-gyp for the installation details.
