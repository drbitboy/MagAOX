#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $DIR/../_common.sh
set -eo pipefail

COMMIT_ISH=dev
orgname=milk-org
reponame=milk
parentdir=/opt/MagAOX/source
if [[ -e $parentdir/$reponame/NO_UPDATES_THANKS ]]; then
  log_info "Lock file at $parentdir/$reponame/NO_UPDATES_THANKS indicates it's all good, no updates thanks"
  cd $parentdir/$reponame
else
  clone_or_update_and_cd $orgname $reponame $parentdir
  git checkout $COMMIT_ISH
  bash -x ./fetch_cacao_dev.sh
fi
sudo rm -rf _build src/config.h src/milk_config.h
mkdir -p _build
cd _build

pythonExe=/opt/conda/bin/python

milkCmakeArgs="-DCMAKE_INSTALL_PREFIX=/usr/local/milk-next -Dbuild_python_module=ON -DPYTHON_EXECUTABLE=${pythonExe}"

if [[ $MAGAOX_ROLE == TIC || $MAGAOX_ROLE == ICC || $MAGAOX_ROLE == RTC || $MAGAOX_ROLE == AOC ]]; then
    milkCmakeArgs="-DUSE_CUDA=ON ${milkCmakeArgs}"
fi

cmake .. $milkCmakeArgs
numCpus=$(nproc)
make -j $((numCpus / 2))
sudo make install

sudo $pythonExe -m pip install -e ../src/ImageStreamIO/
$pythonExe -c 'import ImageStreamIOWrap' || exit 1

milkSuffix=bin/milk
milkBinary=$(grep -e "${milkSuffix}$" ./install_manifest.txt)
milkPath=${milkBinary/${milkSuffix}/}

if command -v milk; then
    log_warn "Found existing milk binary at $(command -v milk)"
fi
link_if_necessary $milkPath /usr/local/milk
echo "/usr/local/milk/lib" | sudo tee /etc/ld.so.conf.d/milk.conf
sudo ldconfig
echo "export PATH=\"\$PATH:/usr/local/milk/bin\"" | sudo tee /etc/profile.d/milk.sh
echo "export PKG_CONFIG_PATH=\$PKG_CONFIG_PATH:/usr/local/milk/lib/pkgconfig" | sudo tee -a /etc/profile.d/milk.sh
echo "export MILK_SHM_DIR=/milk/shm" | sudo tee -a /etc/profile.d/milk.sh

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
if [[ $MAGAOX_ROLE == ICC || $MAGAOX_ROLE == RTC ]]; then
  clone_or_update_and_cd magao-x "cacao-${MAGAOX_ROLE,,}" /data
  link_if_necessary "/data/cacao-${MAGAOX_ROLE,,}" /opt/MagAOX/cacao
  sudo install $DIR/../systemd_units/cacao_startup_if_present.service /etc/systemd/system/
  sudo systemctl daemon-reload || true
  sudo systemctl enable cacao_startup_if_present.service || true
else
  make_on_data_array "cacao-${MAGAOX_ROLE,,}" /opt/MagAOX
fi
log_info "Making /opt/MagAOX/cacao/ owned by xsup:magaox"
chown -R xsup:magaox /opt/MagAOX/cacao/
