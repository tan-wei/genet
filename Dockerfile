FROM node

RUN curl -o- -L https://yarnpkg.com/install.sh | bash
ENV PATH $HOME/.yarn/bin:$PATH
RUN yarn global add electron-download electron-packager electron-winstaller

ARG NPM_CONFIG_ELECTRON_MIRROR
ARG ELECTRON_VERSION=1.5.0
ARG ELECTRON_PLATFORM=darwin

ENV NPM_CONFIG_ELECTRON_MIRROR="https://s3-ap-northeast-1.amazonaws.com/deplug-build-junk/electron/v"

RUN electron-download --platform=$ELECTRON_PLATFORM --version=$ELECTRON_VERSION
