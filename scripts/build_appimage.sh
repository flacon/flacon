
#/bin/bash

set -e

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

cd "${SCRIPT_DIR}/.."

function build() {
    ARCH=$1

    OPTS=""
    OPTS+=" -b"
    OPTS+=" --workflows .github/workflows/build-appimage.yml"
    OPTS+=" --matrix name:${ARCH}"

    if [[ "$ARCH" == "arm64" ]]; then
        OPTS+=" --platform ubuntu-22.04-arm=catthehacker/ubuntu:act-22.04"
        OPTS+=" --container-architecture linux/arm64"
        OPTS+=' --container-options "--cpus=8"'
        #OPTS+=' --container-daemon-opts "--cpus=4"'
    fi

    act $OPTS
}


docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

build x86_64
#build arm64
