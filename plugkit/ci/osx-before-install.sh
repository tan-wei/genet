brew update
brew install gpg jq yarn awscli
nvm install 7.4.0

export PATH=/usr/local/opt/gnupg/libexec/gpgbin:$PATH
export CC=clang
export CXX=clang++
