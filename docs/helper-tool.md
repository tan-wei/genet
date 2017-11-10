# Helper Tool on macOS

## TL;DR:
macOS does not allow normal users to access pcap devices
by default.  Deplug provides a tiny helper tool to handle
this problem. Once it has been installed, we can start a
live capture without running Deplug as root.

## How it works

The helper tool installer will do:

1. Create a new group ‘access_bpf’
2. Put the current user into the group
3. Register a startup script

The startup script runs the following commands as a previliged user every time:

```bash
chgrp access_bpf /dev/bpf*
chmod g+rw /dev/bpf*
```

## Uninstall the helper tool

```bash
sudo launchctl unload /Library/LaunchDaemons/net.deplug.DeplugHelper.plist
sudo rm /Library/LaunchDaemons/net.deplug.DeplugHelper.plist
sudo rm /Library/PrivilegedHelperTools/net.deplug.DeplugHelper
sudo dscl . -delete /Groups/access_bpf
sudo chmod g-rw /dev/bpf*
```
