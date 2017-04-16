aws s3 cp build/Release/plugkit.node s3://deplug-build-junk/plugkit/linux/x64/$(jq .version package.json -r)/plugkit.node --quiet --acl public-read --cache-control "max-age=3600"
