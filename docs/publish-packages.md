# Publish

## Pre-built binaries

You can distribute pre-built binaries for native packages.
Put a `deplug.binary.souece` entry into `package.json` to indicate the download source.

```json
{
    "name": "package_name",
    "version": "1.0.0",
    "deplug": {
        "binary": {
            "source": "https://deplug.net/prebuilt/"
        }
    }
}
```

The name of the archive is represented as
`${this.pkg.name}-v${this.pkg.version}-abi${process.versions.modules}-${process.platform}-${process.arch}.zip`.

For example, on Linux with node@8.1.2 (ABI version 57), the package installer will try to download
`https://deplug.net/prebuilt/package_name-v1.0.0-abi57-linux-x64.zip` and extract it into the directory contains `package.json`.

If the archive is not available, the installer will build the native package with `node-gyp`.

Using pre-built binaries can be disabled by setting `_.noPrebuilt` to `true` in `config.yml`.
