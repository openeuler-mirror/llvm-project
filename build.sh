#!/bin/bash

# Tools to use for bootstrapping.
C_COMPILER_PATH=gcc
CXX_COMPILER_PATH=g++

# Initialize our own variables:
buildtype=RelWithDebInfo
backends="ARM;AArch64;X86"
split_dwarf=on
use_ccache="0"
do_install="0"
clean=0
unit_test=""
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

Build the compiler under $(get_relative_path $build_prefix), then install under $(get_relative_path $install_prefix) .

Options:
  -b type  Specify CMake build type (default: $buildtype).
  -c       Use ccache (default: $use_ccache).
  -h       Display this help message.
  -i       Install the build (default: $do_install).
  -I name  Specify install directory name (default: "$install_dir_name").
  -j N     Allow N jobs at once (default: $threads).
  -r       Delete $(get_relative_path $install_prefix) and perform a clean build (default: incremental).
  -t       Enable unit tests for components that support them (make check-all).
  -v       Enable verbose build output (default: quiet).
  -X archs Build only the specified semi-colon-delimited list of backends (default: "$backends").
EOF
}

# Process command-line options. Remember the options for passing to the
# containerized build script.
while getopts :b:chiI:j:rtvX: optchr; do
  case "$optchr" in
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
      ;;
    c)
      use_ccache="1"
      ;;
    h)
      usage
      exit
      ;;
    i)
      do_install="1"
      ;;
    I)
      install_dir_name="$OPTARG"
      install_prefix="$dir/$install_dir_name"
      ;;
    j)
      threads="$OPTARG"
      ;;
    r)
      clean=1
      ;;
    t)
      unit_test=check-all
      ;;
    v)
      verbose="VERBOSE=1"
      ;;
    X)
      backends="$OPTARG"
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

CMAKE_OPTIONS="-DCMAKE_INSTALL_PREFIX=$install_prefix \
               -DCMAKE_BUILD_TYPE=$buildtype \
               -DCMAKE_C_COMPILER=$C_COMPILER_PATH \
               -DCMAKE_CXX_COMPILER=$CXX_COMPILER_PATH \
               -DLLVM_TARGETS_TO_BUILD=$backends "

# Warning: the -DLLVM_ENABLE_PROJECTS option is specified with cmake
# to avoid issues with nested quotation marks

if [ $use_ccache == "1" ]; then
  echo "Build using ccache"
  CMAKE_OPTIONS="$CMAKE_OPTIONS \
                -DCMAKE_C_COMPILER_LAUNCHER=ccache \
                -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "
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
      -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;libunwind;lld;openmp;clang-tools-extra" \
      -DLLVM_USE_LINKER=gold \
      -DLLVM_LIT_ARGS="-sv -j$threads" \
      -DLLVM_USE_SPLIT_DWARF=$split_dwarf \
      -DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="-Wl,--gdb-index -Wl,--compress-debug-sections=zlib"\
      -DCMAKE_EXE_LINKER_FLAGS_DEBUG="-Wl,--gdb-index -Wl,--compress-debug-sections=zlib" \
      $verbose \
      ../llvm

make -j$threads
if [ $do_install == "1" ]; then
  make -j$threads $verbose install
fi

if [ -n "$unit_test" ]; then
  make -j$threads $verbose check-all
fi

cd ..

echo "$0: SUCCESS"
