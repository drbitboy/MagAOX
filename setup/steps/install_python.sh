#!/bin/bash
# If not started as root, sudo yourself
if [[ "$EUID" != 0 ]]; then
    sudo bash -l $0 "$@"
    exit $?
fi
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $DIR/../_common.sh
set -euo pipefail
MINICONDA_VERSION="3-py38_4.8.3"
#
# MINICONDA
#
cd /opt/MagAOX/vendor
if [[ ! -d /opt/miniconda3 ]]; then
    MINICONDA_INSTALLER="Miniconda$MINICONDA_VERSION-Linux-x86_64.sh"
    _cached_fetch "https://repo.continuum.io/miniconda/$MINICONDA_INSTALLER" $MINICONDA_INSTALLER
    bash $MINICONDA_INSTALLER -b -p /opt/miniconda3
	# Ensure magaox-dev can write to /opt/miniconda3 or env creation will fail
	chown -R :magaox-dev /opt/miniconda3
    # set group and permissions such that only magaox-dev has write access
    chmod -R g=rwX /opt/miniconda3
    find /opt/miniconda3 -type d -exec sudo chmod g+rwxs {} \;
    # Backup plan: use ACLs
    # sudo setfacl -Rm g:magaox-dev:rwX,d:g:magaox-dev:rwX /opt/miniconda3
    # Set environment variables for miniconda
    cat << 'EOF' | tee /etc/profile.d/miniconda.sh
if [ -f "/opt/miniconda3/etc/profile.d/conda.sh" ]; then
    . "/opt/miniconda3/etc/profile.d/conda.sh"
    CONDA_CHANGEPS1=false conda activate base
else
    \export PATH="/opt/miniconda3/bin:$PATH"
fi
EOF
    cat << 'EOF' | tee /opt/miniconda3/.condarc
channels:
  - conda-forge
  - defaults
changeps1: false
channel_priority: flexible
EOF
fi
