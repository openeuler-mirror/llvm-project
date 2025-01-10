#!/bin/bash

# Tools to use for bootstrapping.
C_COMPILER_PATH=gcc
CXX_COMPILER_PATH=g++

# Initialize our own variables:
enable_acpo="1"
enable_autotuner="1"
buildtype=RelWithDebInfo
backends="all"
build_for_openeuler="0"
enabled_projects="clang;lld;compiler-rt;openmp;clang-tools-extra"
embedded_toolchain="0"
split_dwarf=on
use_ccache="0"
enable_classic_flang="0"
do_install="0"
clean=0
containerize=0
docker=$(type -p docker)
host_arch="$(uname -m)"
unit_test=""
install="install"
install_toolchain_only="0"
verbose=""
dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
build_dir_name="build"
install_dir_name="install"
build_prefix="$dir/$build_dir_name"
install_prefix="$dir/$install_dir_name"

# Use 8 threads for builds and tests by default. Use more threads if possible,
# but avoid overloading the system by using up to 50% of available cores.
threads=8
nproc=$(type -p nproc)
if [ -x "$nproc" -a -f /proc/loadavg ]; then
  loadavg=$(awk '{printf "%.0f\n", $1}' < /proc/loadavg)
  let threads="($($nproc) - $loadavg) / 2"
  if [ $threads -le 0 ]; then
    threads=1
  fi
fi

# Exit script on first error.
set -e

usage() {
  cat <<EOF
Usage: $0 [options]

Build the compiler under $build_prefix, then install under $install_prefix.

Options:
  -a       Disable BiSheng-Autotuner.
  -A       Disable ACPO.
  -b type  Specify CMake build type (default: $buildtype).
  -c       Use ccache (default: $use_ccache).
  -C       Containerize the build for openEuler compatibility.
  -e       Build for embedded cross tool chain.
  -E       Build for openEuler.
  -h       Display this help message.
  -i       Install the build (default: $do_install).
  -I name  Specify install directory name (default: "$install_dir_name").
  -d dir   Specify the build directory (default: "$build_dir_name").
  -j N     Allow N jobs at once (default: $threads).
  -o       Enable LLVM_INSTALL_TOOLCHAIN_ONLY=ON.
  -r       Delete $install_prefix and perform a clean build (default: incremental).
  -s       Strip binaries and minimize file permissions when (re-)installing.
  -t       Enable unit tests for components that support them (make check-all).
  -v       Enable verbose build output (default: quiet).
  -f       Enable classic flang.
  -X archs Build only the specified semi-colon-delimited list of backends (default: "$backends").
EOF
}

# Process command-line options. Remember the options for passing to the
# containerized build script.
containerized_opts=()
while getopts :aAb:d:cCeEhiI:j:orstvfX: optchr; do
  case "$optchr" in
    a)
      enable_autotuner="0"
      containerized_opts+=(-$optchr)
      ;;
    A)
      enable_acpo="0"
      containerized_opts+=(-$optchr)
      ;;
    b)
      buildtype="$OPTARG"
      case "${buildtype,,}" in
        release)
          split_dwarf=off
          ;;
        debug|relwithdebinfo)
          ;;
        *)
          echo "$0: invalid build type '$buildtype'"
          exit 1
          ;;
      esac
      containerized_opts+=(-$optchr "$OPTARG")
      ;;
    c)
      use_ccache="1"
      containerized_opts+=(-$optchr)
      ;;
    C)
      if [ -z "$docker" -o ! -x "$docker" ]; then
        echo "$0: no usable Docker"
        exit 1
      fi
      containerize=1
      ;;
    d)
      build_prefix="$OPTARG"
      containerized_opts+=(-$optchr "$OPTARG")
      ;;
    f)
      enable_classic_flang="1"
      containerized_opts+=(-$optchr)
      ;;
    e)
      embedded_toolchain="1"
      containerized_opts+=(-$optchr)
      ;;
    E)
      build_for_openeuler="1"
      containerized_opts+=(-$optchr)
      ;;
    h)
      usage
      exit
      ;;
    i)
      do_install="1"
      containerized_opts+=(-$optchr)
      ;;
    I)
      install_dir_name="$OPTARG"
      install_prefix="$dir/$install_dir_name"
      containerized_opts+=(-$optchr "$OPTARG")
      ;;
    j)
      threads="$OPTARG"
      containerized_opts+=(-$optchr "$OPTARG")
      ;;
    o)
      install_toolchain_only=1
      containerized_opts+=(-$optchr)
      ;;
    r)
      clean=1
      containerized_opts+=(-$optchr)
      ;;
    s)
      install="install/strip"
      containerized_opts+=(-$optchr)
      ;;
    t)
      unit_test=check-all
      containerized_opts+=(-$optchr)
      ;;
    v)
      verbose="VERBOSE=1"
      containerized_opts+=(-$optchr)
      ;;
    X)
      backends="$OPTARG"
      containerized_opts+=(-$optchr "$OPTARG")
      ;;
    :)
      echo "$0: missing argument for option '-$OPTARG'"
      exit 1
      ;;
    ?)
      echo "$0: invalid option '-$OPTARG'"
      exit 1
      ;;
  esac
done

# Make sure that all files under the build directory can be deleted; when some
# LLVM tests are interrupted, they can leave behind inaccessible directories.
build_cleanup() {
  chmod -R u+rwX,go+rX $build_prefix > /dev/null 2>&1
}

# Handle interrupts. When not running a containerized build, we have to enable
# job control (-m), and make sure to delete our own long-running child
# processes. In particular, ninja and python (llvm-lit) refuse to terminate
# when Jenkins aborts the parent process and disconnects.
set -m
handle_abort() {
  local rc=$1 sig=$2
  build_cleanup
  trap - EXIT SIGHUP SIGINT SIGTERM
  local children="$(jobs -p)"
  for cgrp in $children ; do
    kill -$sig -${cgrp} > /dev/null 2>&1
  done
  if [ -n "$sig" ]; then
    kill -$sig 0
  else
    exit $rc
  fi
}

if [ $containerize -eq 0 ]; then
  trap - SIGCHLD
  trap 'handle_abort $?' EXIT
  trap 'handle_abort 129 HUP' SIGHUP
  trap 'handle_abort 130 INT' SIGINT
  trap 'handle_abort 143 TERM' SIGTERM
else
  cmd=$(readlink --canonicalize-existing $0)

  # Generate passwd/group files for the container.
  # lit.py depends on correct results from getpwent.
  homedir=$(realpath $HOME)
  passwd=$(mktemp /tmp/passwd.XXXXXX)
  echo "root:x:0:0::/root:/bin/bash" > $passwd
  echo "user:x:$(id -u):$(id -g)::$homedir:/bin/bash" >> $passwd

  group=$(mktemp /tmp/group.XXXXXX)
  echo "root:x:0:" > $group
  echo "users:x:$(id -g):" >> $group

  # Re-run myself in a openEuler container. The --cap-add option is needed
  # to pacify llvm-exegesis unit tests. Make sure that the container is
  # stopped if the child process is interrupted (e.g. by Jenkins).
  # Note that the container ID file is created with `mktemp -u` because
  # `docker run` refuses to overwrite an existing ID file.
  echo "Re-launching in container."
  containerid=$(mktemp -u /tmp/docker-cid.$$.XXXXXX)
  docker_cleanup() {
    local rc=$1 sig=$2
    docker container stop $(cat $containerid) > /dev/null 2>&1
    rm $passwd $group $containerid
    build_cleanup
    trap - EXIT SIGHUP SIGINT SIGTERM # avoid infinite recursion
    if [ -n "$sig" ]; then
      kill -$sig 0
    else
      exit $rc
    fi
  }
  trap 'docker_cleanup $?' EXIT
  trap 'docker_cleanup 129 HUP' SIGHUP
  trap 'docker_cleanup 130 INT' SIGINT
  trap 'docker_cleanup 143 TERM' SIGTERM

  DOCKER_IMAGE="llvm-build-deps:latest"
  docker_opts="--rm
    --cap-add=SYS_ADMIN
    --cap-add=SYS_PTRACE
    --security-opt seccomp=unconfined
    --user $(id -u):$(id -g)
    --workdir=$(realpath $PWD)
    --ulimit core=0
    --ulimit stack=-1
    --cidfile $containerid
    -v $homedir:$homedir
    -v $passwd:/etc/passwd
    -v $group:/etc/group
    -e BINUTILS_INCDIR=/usr/local/include
    hub.oepkgs.net/openeuler/${DOCKER_IMAGE}"

  if [ -t 1 ]; then
    $docker run -it $docker_opts ${cmd} ${containerized_opts[@]}
    exit $?
  else
    set -x
    $docker run $docker_opts ${cmd} ${containerized_opts[@]} &
    wait $! || exit $?
    exit 0
  fi
fi

echo "Using $threads threads."

CMAKE_OPTIONS="-DCMAKE_INSTALL_PREFIX=$install_prefix \
               -DCMAKE_BUILD_TYPE=$buildtype \
               -DCMAKE_C_COMPILER=$C_COMPILER_PATH \
               -DCMAKE_CXX_COMPILER=$CXX_COMPILER_PATH \
               -DLLVM_TARGETS_TO_BUILD=$backends "

gold=$(type -p ld.gold)
if [ -z "$gold" -o ! -x "$gold" ]; then
  echo "$0: no usable ld.gold"
  exit 1
fi

# If the invocation does not force a particular binutils installation, check
# that we are using an acceptable version.
if [ -n "$BINUTILS_INCDIR" ]; then
  llvm_binutils_incdir="-DLLVM_BINUTILS_INCDIR=$BINUTILS_INCDIR"
else
  incdir=$(realpath --canonicalize-existing $(dirname $gold)/../include)
  if [ -z "$incdir" -o ! -f "$incdir/plugin-api.h" ]; then
    echo "$0: plugin-api.h not found; required to build LLVMgold.so"
    exit 1
  fi
  llvm_binutils_incdir="-DLLVM_BINUTILS_INCDIR=$incdir"
fi

# Warning: the -DLLVM_ENABLE_PROJECTS option is specified with cmake
# to avoid issues with nested quotation marks
if [ $use_ccache == "1" ]; then
  echo "Build using ccache"
  CMAKE_OPTIONS="$CMAKE_OPTIONS \
                -DCMAKE_C_COMPILER_LAUNCHER=ccache \
                -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "
fi

if [ $enable_classic_flang == "1" ]; then
  echo "Enable classic flang"
  CMAKE_OPTIONS="$CMAKE_OPTIONS \
                -DLLVM_ENABLE_CLASSIC_FLANG=on"
fi

if [ $embedded_toolchain == "1" ]; then
  echo "Build for embedded cross tool chain"
  enabled_projects="clang;lld;compiler-rt;"
  CMAKE_OPTIONS="$CMAKE_OPTIONS \
                -DLLVM_BUILD_FOR_EMBEDDED=ON"
fi

# When set LLVM_INSTALL_TOOLCHAIN_ONLY to On it removes many of the LLVM development
# and testing tools as well as component libraries from the default install target.
if [ $install_toolchain_only == "1" ]; then
  echo "Only install toolchain"
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON"
fi

if [ $build_for_openeuler == "1" ]; then
  echo "Build for openEuler"
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_FOR_OPENEULER=ON"
fi

if [ $enable_autotuner == "1" ]; then
  echo "enable BiSheng-Autotuner"
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DLLVM_ENABLE_AUTOTUNER=ON"
fi

if [ $enable_acpo == "1" ]; then
  echo "enable ACPO"
  export CFLAGS="-Wp,-DENABLE_ACPO ${CFLAGS}"
  export CXXFLAGS="-Wp,-DENABLE_ACPO ${CXXFLAGS}"
fi

# Build and install
if [ $clean -eq 1 -a -e "$install_prefix" ]; then
  rm -rf "$install_prefix"
fi
mkdir -p "$install_prefix/bin"

if [ $clean -eq 1 -a -e "$build_prefix" ]; then
  rm -rf "$build_prefix"
fi

mkdir -p "$build_prefix" && cd "$build_prefix"
cmake $CMAKE_OPTIONS \
      -DCOMPILER_RT_BUILD_SANITIZERS=on \
      -DLLVM_ENABLE_PROJECTS=$enabled_projects \
      -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
      -DLLVM_USE_LINKER=gold \
      -DLLVM_LIT_ARGS="-sv -j$threads" \
      -DLLVM_USE_SPLIT_DWARF=$split_dwarf \
      -DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="-Wl,--gdb-index -Wl,--compress-debug-sections=zlib" \
      -DCMAKE_EXE_LINKER_FLAGS_DEBUG="-Wl,--gdb-index -Wl,--compress-debug-sections=zlib" \
      -DBUILD_SHARED_LIBS=OFF \
      -DLLVM_ENABLE_LIBCXX=OFF \
      -DLLVM_ENABLE_ZLIB=ON \
      -DLLVM_BUILD_RUNTIME=ON \
      -DLLVM_INCLUDE_TOOLS=ON \
      -DLLVM_BUILD_TOOLS=ON \
      -DLLVM_INCLUDE_TESTS=ON \
      -DLLVM_BUILD_TESTS=ON \
      -DLLVM_INCLUDE_EXAMPLES=ON \
      -DLLVM_BUILD_EXAMPLES=OFF \
      -DCLANG_DEFAULT_PIE_ON_LINUX=ON \
      -DCLANG_ENABLE_ARCMT=ON \
      -DCLANG_ENABLE_STATIC_ANALYZER=ON \
      -DCLANG_PLUGIN_SUPPORT=ON \
      -DLLVM_DYLIB_COMPONENTS="all" \
      -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON \
      -DCMAKE_SKIP_RPATH=ON \
      -DLLVM_ENABLE_FFI=ON \
      -DLLVM_ENABLE_RTTI=ON \
      -DLLVM_USE_PERF=ON \
      -DLLVM_INSTALL_GTEST=ON \
      -DLLVM_INCLUDE_UTILS=ON \
      -DLLVM_INSTALL_UTILS=ON \
      -DLLVM_INCLUDE_BENCHMARKS=OFF \
      -DENABLE_LINKER_BUILD_ID=ON \
      -DLLVM_ENABLE_EH=ON \
      -DCLANG_DEFAULT_UNWINDLIB=libgcc \
      -DLIBCXX_STATICALLY_LINK_ABI_IN_STATIC_LIBRARY=ON \
      -DLIBCXX_ENABLE_ABI_LINKER_SCRIPT=ON \
      -DLIBOMP_INSTALL_ALIASES=OFF \
      $llvm_binutils_incdir \
      $verbose \
      ../llvm

make -j$threads
if [ $do_install == "1" ]; then
  make -j$threads $verbose $install
fi

if [ -n "$unit_test" ]; then
  make -j$threads $verbose check-all
fi

cd ..

# When building official deliverables, minimize file permissions under the
# installation directory.
if [ "$install" = "install/strip" ]; then
  find $install_prefix/bin/ -type f -exec strip {} \;
  find $install_prefix -type f -exec chmod a-w {} \;
fi

# In openEuler embedded building system, it need wrap llvm-readelf
# to replace binutils-readelf.
if [ -e "$install_prefix/bin/llvm-readobj" ]; then
  ln -sf llvm-readobj $install_prefix/bin/llvm-readelf
fi

echo "$0: SUCCESS"
