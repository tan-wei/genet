# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]

## [0.4.0] - 2018-09-03
### Added
- Add pacman distribution.

### Changed
- KeyBind: Treat a command-key as a ctrl-key on Linux/Windows.
- Add the genet-abi verison in .version file.
- ARP: add _.src and _.dst attribute.

## [0.3.0] - 2018-08-19
### Added
- Integrate gpm (genet package manager).
- Add tab keybinds.
- Preference: Show ABI version.
- Preference: Add License tab.

### Changed
- Rename Decoder to Cast.
- Rename Dissector to Decoder.

## [0.2.0] - 2018-08-10
### Added
- Add resumable reloading.
- Publish genet-abi@0.1.0 and genet-cargo@0.1.0 on crates.io.
- Show chrome devtools when window.onerror is triggered.
- Add the automatic tab reloading by touching $HOME/.genet/.reload.

### Changed
- InputDialog: Show special message if there is no input source.
- Show thread panic to the gui notification instead of stdout.
- Change the format of the .version file.
- Better default layer summary.
- Dialog: Add the genet accent color.
- Preferences: Use the schema id as a title.
- Pcap: Move built-in pcap preferences to pcap package preferences.
- Use the cargo workspace for genet-kernel and dependencies.
- Merge @genet/attr-types and @genet/tokens into @genet/base.
- TokenComponent: Accept true value instead of {}.
- TokenComponent: change config definition.
- StyleComponent: change config definition.
- PackageManager: Report errors from ComponentFactory.create.
- DisplayFilter: Show filter syntax errors right under the filter input.

### Removed
- Remove unused tokens.
- Remove _.package.rustPath config.

### Fixed
- Fix an invalid range causing panic in Store#frames, Store#filtered_frames.
- PermissionMassage: Fix the message for linux.
- Fix an invalid display filter causing a crash in genet-node.

## 0.1.0 - 2018-08-03
- Initial Release

[Unreleased]: https://github.com/genet-app/genet/compare/v0.4.0...HEAD
[0.4.0]: https://github.com/genet-app/genet/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/genet-app/genet/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/genet-app/genet/compare/v0.1.0...v0.2.0
