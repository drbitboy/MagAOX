#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $DIR/../_common.sh
set -euo pipefail

if [[ $MAGAOX_ROLE == RTC ||$MAGAOX_ROLE == ICC || $MAGAOX_ROLE == AOC || $MAGAOX_ROLE == TIC || $MAGAOX_ROLE == ci ]]; then
  CMAKE_FLAGS="-DUSE_CUDA=YES -DUSE_MAGMA=YES"
else
  CMAKE_FLAGS=""
fi

CACAO_DIR=/opt/MagAOX/source/cacao

cd /opt/MagAOX/source

if [[ ! -d $CACAO_DIR ]]; then
    git clone --recursive --branch dev https://github.com/magao-x/cacao.git cacao
    cd ./cacao
    git config core.sharedRepository true
    git remote add upstream https://github.com/cacao-org/cacao.git
else
    log_info "Not touching cacao since it's already present"
    cd ./cacao
fi
CACAO_ABSPATH=$PWD

mkdir -p _build
cd _build
if [[ ! -e Makefile ]]; then
  cmake ../ $CMAKE_FLAGS
fi
make
sudo make install
if [[ ! -e /usr/local/bin/milk ]]; then
    sudo ln -s /usr/local/bin/cacao /usr/local/bin/milk
fi
echo "export PATH=\$PATH:$CACAO_DIR/src/CommandLineInterface/scripts" | sudo tee /etc/profile.d/cacao.sh
echo "export MILK_SHM_DIR=/milk/shm" | sudo tee -a /etc/profile.d/cacao.sh

if [[ $MAGAOX_ROLE != ci ]]; then
  if ! grep -q "/milk/shm" /etc/fstab; then
    echo "tmpfs /milk/shm tmpfs rw,nosuid,nodev" | sudo tee -a /etc/fstab
    sudo mkdir -p /milk/shm
    log_success "Created /milk/shm tmpfs mountpoint"
    sudo mount /milk/shm
    log_success "Mounted /milk/shm"
  else
    log_info "Skipping /milk/shm mount setup"
  fi
fi
