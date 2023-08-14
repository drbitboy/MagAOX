#!/usr/bin/env bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $DIR/../_common.sh
if [[ ! $ID == "ubuntu" ]]; then
    log_error "Only installing from package manager on Ubuntu"
    exit 1
fi
sudo NEEDRESTART_SUSPEND=yes apt install -y intel-oneapi-mkl-devel-2023.2.0 || error_exit "Couldn't install MKL"
echo "source /opt/intel/oneapi/mkl/latest/env/vars.sh intel64" | sudo tee /etc/profile.d/mklvars.sh &>/dev/null || exit 1
echo "/opt/intel/oneapi/mkl/latest/lib/intel64" | sudo tee /etc/ld.so.conf.d/mkl.conf || exit 1
sudo ldconfig
