#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $DIR/../_common.sh
set -euo pipefail
XRIF_COMMIT="9a38146c4a63f70372d2ffe545f80c6aad696808"
XRIF_DIR="./xrif"

#
# xrif streaming compression library
#

if [[ ! -d $XRIF_DIR ]]; then
    git clone https://github.com/jaredmales/xrif.git $XRIF_DIR
fi
cd $XRIF_DIR
git config core.sharedRepository group
git checkout $XRIF_COMMIT
mkdir -p build
cd build
cmake ..
make
make test
make install
ldconfig
