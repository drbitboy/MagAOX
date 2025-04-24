#!/usr/bin/env bash

#################################################################################
# perform an unattended multipass VM setup and provisioning for MagAO-X
#
# note that you need ~/.ssh/id_ed25519.pub
#
# todo: provide CLI options to change disk/cpus/memory
function default_options() {
  one="$1"
  cat << __EoF | ( [ "$one" ] && cat || ( sed 's/ *#.*//'| tr \\n ' ' ) )
### Command-line options, with defaults:
-i=24.04                ### VM instance
-c=4                    ### VM CPUs
-d=20GiB                ### VM disk space
-m=8.0GiB               ### VM memory
-v=magao-x-vm           ### Multipass VM name
-M=magao-x/MagAOX,,dev  ### MagAOX:  GithubUsr/GithubName,TargDir,Branch
-r=                     ### Additional repos
-SP=                    ### Skip Provisioning if not empty
-2=                     ### Duplicate VM, cloned from first (see -v=...)
-k=${HOME}/.ssh/id_ed25519.pub      ### SSH public key
__EoF
}
for arg in $(default_options) $* ; do
  [ "$arg" ] || continue
  case "$arg" in
  -i=*) _arg_i=${arg#-i=} ;;
  -c=*) _arg_c=${arg#-c=} ;;
  -d=*) _arg_d=${arg#-d=} ;;
  -m=*) _arg_m=${arg#-m=} ;;
  -v=*) _arg_v=${arg#-v=} ;;
  -M=*,*,*) _arg_M=${arg#-M=} ;;
  -r=*,*,*) _arg_r="${_arg_r} ${arg#-r=}" ;;
  -r=) true ;;
  -SP=*) _arg_SP="${arg#-SP=}" ;;
  -2=*) _arg_2=${arg#-2=} ;;
  -k=*) _arg_k=${arg#-k=} ;;
  *) echo "Bad argument [$arg]; exiting" && false || default_options help && false || exit 1 ;;
  esac
done
#
################################################################################

#VM name
vmname=${_arg_v}

#Source Github repo for MagAOX
MagAOX_userATrepo=${_arg_M%%,*}
_M=${_arg_M#$MagAOX_userATrepo,}
MagAOX_subdir=${_M%%,*}
[ "$MagAOX_subdir" ] || MagAOX_subdir=MagAOX
MagAOX_branch=${_M##*,}
[ "$MagAOX_branch" ] && MagAOX_branch="-b $MagAOX_branch"

set | grep -E '_arg_..*=|^MagAOX_'

#basic VM creation
multipass launch -n $vmname -c $_arg_c -d $_arg_d -m $_arg_m $_arg_i


#ensure public key path ends in .pub
[ "${_arg_k%.pub}" == "${_arg_k}" ] \
&& _arg_k="$_arg_k.pub" || true

#ensure public key exists
[ -r "$_arg_k" ] || ssh-keygen -t ed25519 -N "" -f "${_arg_k%.pub}"

#install our key
multipass exec $vmname -- bash -c "echo $(cat $_arg_k) >> ~/.ssh/authorized_keys"

#user@IP
uATip=ubuntu@$(multipass exec $vmname -- hostname -I | awk '{print $1}')

#Ensure this IP is not in known hosts file
ssh-add -f "${HOME}/.ssh/known_hosts" -R "${uATip#ubuntu@}" 2>/dev/null || true

# first ssh needs to force acceptance of the host key
ssh -o StrictHostKeyChecking=accept-new $uATip "sudo apt update && sudo apt upgrade -y"

#apply the updates
multipass stop $vmname
multipass start $vmname

ssh $uATip "mkdir githubalt"

#Clone MagAO-X repo into VM
ssh $uATip "git clone --depth=1 $MagAOX_branch https://github.com/${MagAOX_userATrepo}.git githubalt/$MagAOX_subdir"

for uATr_d_b in $_arg_r ; do
  uATr=${uATr_d_b%%,*}
  _r=${uATr_d_b#${uATr},}
  subdir=${_r%%,*}
  branch=${_r##*,}
  [ "$branch" ] && branch="-b $branch"
  ssh $uATip "git clone --depth=1 $branch https://github.com/${uATr}.git githubalt/$subdir"
done

#pre_provision as workstation to setup environment and users & groups
ssh $uATip "cd githubalt/MagAOX/setup && MAGAOX_ROLE=workstation ./pre_provision.sh"

#this must be a separate login to get groups updated
[ "$_arg_SP" ] \
&& ( echo Skipping provisioning ... || true ) \
|| ssh $uATip "cd githubalt/MagAOX/setup && bash ./provision.sh"

changerole=/opt/MagAOX/config/change_role_to_hostname.sh

vmname2=$_arg_2

#clone second multipass VM if requested via -2=vmname2
if [ "$vmname2" ] ; then
  #stop first VM, clone to second VM, start both VMs
  multipass stop $vmname
  multipass clone -n $vmname2 $vmname
  multipass start $vmname $vmname2
  #get ubuntu@IP address of second VM
  uATip2=ubuntu@$(multipass exec $vmname2 -- hostname -I | awk '{print $1}')
  #clear second VM's IP address from known hosts
  ssh-add -f "${HOME}/.ssh/known_hosts" -R "${uATip2#ubuntu@}" 2>/dev/null || true
  #change role of second VM
  ssh -o StrictHostKeyChecking=accept-new $uATip2 "[ -x '$changerole' ] && $changerole || true"
fi

#change role of first VM
ssh $uATip "[ -x '$changerole' ] && $changerole || true"
