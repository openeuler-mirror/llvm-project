#!/bin/bash
set +e

# 1. 环境信息采集
uname -a

# 2. 依赖包安装
sudo yum install -y git
sudo yum install -y g++
sudo yum install -y libgcc
sudo yum install -y openssl-devel
sudo yum install -y cmake
sudo yum install -y make
sudo yum install -y python3
sudo yum install -y python3-setuptools
sudo yum install -y python-wheel
sudo yum install -y texinfo
sudo yum install -y binutils
sudo yum install -y binutils-devel
sudo yum install -y libstdc++-static
sudo yum install -y libatomic
sudo yum install -y libasan
sudo yum install -y libubsan
sudo yum install -y liblsan
sudo yum install -y texinfo
sudo yum install -y tar
sudo yum install -y nfs-utils
sudo yum install -y dejagnu
sudo yum install -y jq
# 来源自src-openeuler/llvm里的BuildRequires
sudo yum install -y gcc gcc-c++ clang cmake chrpath ninja-build zlib-devel libzstd-devel libffi-devel ncurses-devel binutils-devel libedit-devel multilib-rpm-config python3-devel python3-psutil python3-sphinx python3-setuptools libedit-devel swig libxml2-devel doxygen elfutils-libelf-devel perl perl-Data-Dumper perl-Encode libffi-devel perl-generators emacs libatomic python3-numpy python3-pybind11 python3-pyyaml graphviz procps-ng
# feature-thinlto-split分支依赖numactl-devel包
sudo yum install -y numactl-devel

# 3. 过滤失败用例
if [ "$(arch)" == "aarch64" ]; then
  source aarch64_failure.sh
elif [ "$(arch)" == "x86_64" ]; then
  source x86_64_failure.sh
fi

# 4. 执行构建脚本
#   4.1 定义变量
PR_ID=${prid}
SOURCE_BRANCH=${branch}
DEST_BRANCH=${tbranch}
AUTHOR=${committer}
PARALLEL_JOBS="16"

if [ "$(arch)" == "aarch64" ]; then
  if [ "${DEST_BRANCH}" == "dev_17.0.6" ]; then
    PARALLEL_JOBS="32"
  fi
elif [ "$(arch)" == "x86_64" ]; then
  PARALLEL_JOBS="32"
fi

#   4.2 执行构建
cd ${WORKSPACE}/llvm-project
bash build.sh -v -r -t -b relwithdebinfo -E -i -I install-$(arch)-for-pr-${PR_ID} -j${PARALLEL_JOBS}
RESULT=$?
exit ${RESULT}
