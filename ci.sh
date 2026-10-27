#!/bin/bash
# ci.sh — openEuler llvm-project PR 构建 CI 门禁入口（单文件）。
#
# 流程:
#   1. 环境信息采集
#   2. 依赖包安装
#   3. 加载已知失败用例过滤（failures.json -> LIT_FILTER_OUT，按 tbranch+arch 匹配）
#   4. PR 缓存检测（已通过且无变化则跳过构建，可用 SKIP_IF_PASSED 关闭）
#   5. 调用 build.sh 构建 + 跑单测
#   6. 结果回写 pr_status 仓库
#
# 依赖（由 CI/Jenkins 注入的环境变量）:
#   WORKSPACE  工作区根目录
#   prid       待测 PR 编号
#   branch     PR 源分支
#   tbranch    PR 目标分支
#   committer  PR 提交者
#
# 可选:
#   SKIP_IF_PASSED  默认 1；设为 0 强制重测（忽略 PR 已通过缓存）
#   PR_STATUS_TOKEN 访问 pr_status 仓库的令牌；也可用命令行参数 $1 传入
#
# 用法:
#   ci.sh [PR_STATUS_TOKEN]

set +e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
FAILURES_CONF="${SCRIPT_DIR}/failures.conf"

# ---- 日志工具 ------------------------------------------------------------

log_step()  { printf '\033[1;32m[STEP]\033[0m %s\n' "$*"; }
log_info()  { printf '\033[0;36m[INFO]\033[0m %s\n' "$*"; }
log_warn()  { printf '\033[0;33m[WARN]\033[0m %s\n' "$*"; }
log_error() { printf '\033[0;31m[ERROR]\033[0m %s\n' "$*" >&2; }

# ---- 1. 环境信息采集 -----------------------------------------------------

log_step "environment information"
uname -a
ARCH="$(uname -m)"
log_info "arch=${ARCH}"

# ---- 2. 依赖包安装 -------------------------------------------------------

log_step "installing dependencies"

# 基础依赖
BASE_DEPS=(git g++ libgcc openssl-devel cmake make python3 python3-setuptools \
  python-wheel texinfo binutils binutils-devel libstdc++-static libatomic \
  libasan libubsan liblsan tar nfs-utils dejagnu jq numactl-devel)

# 来源自 src-openeuler/llvm 里的 BuildRequires
SRC_DEPS=(gcc gcc-c++ clang cmake chrpath ninja-build zlib-devel libzstd-devel \
  libffi-devel ncurses-devel binutils-devel libedit-devel multilib-rpm-config \
  python3-devel python3-psutil python3-sphinx python3-setuptools libedit-devel \
  swig libxml2-devel doxygen elfutils-libelf-devel perl perl-Data-Dumper \
  perl-Encode libffi-devel perl-generators emacs libatomic python3-numpy \
  python3-pybind11 python3-pyyaml graphviz procps-ng)

log_info "installing base + src-openeuler packages"
sudo yum install -y "${BASE_DEPS[@]}" "${SRC_DEPS[@]}"

# ---- 3. 已知失败用例过滤 -------------------------------------------------

log_step "loading known-failure filters"

if [ ! -f "${FAILURES_CONF}" ]; then
  log_warn "failures.conf missing; skip test filtering"
  LIT_FILTER_OUT=""
else
  # 解析 INI 格式 failures.conf。
  # 第一步：定位 tbranch 所在分组 KEY（精确 / 逗号包含 / 回退 [*]）。
  log_info "filter config: tbranch='${tbranch}' arch='${ARCH}'"
  KEY=""
  while IFS= read -r line; do
    case "${line}" in
      '['*']')
        grp=${line#[}; grp=${grp%]}
        if [ "${grp}" = "${tbranch}" ]; then
          KEY="${grp}"; break
        fi
        case ",${grp}," in
          *",${tbranch},"*) KEY="${grp}"; break ;;
        esac
        ;;
    esac
  done < "${FAILURES_CONF}"
  if [ -z "${KEY}" ]; then
    log_warn "branch '${tbranch}' not matched in failures.conf; falling back to [*]"
    KEY="*"
  else
    log_info "branch '${tbranch}' matched group '${KEY}'"
  fi

  # 第二步：从 KEY 分组和 [*] 分组收集 arch 块下的用例行。
  # KEY=[*] 时只需扫描一次。
  LIT_FILTER_OUT=""
  FILTER_COUNT=0
  if [ "${KEY}" = "*" ]; then
    groups=("*")
  else
    groups=("${KEY}" "*")
  fi
  for grp in "${groups[@]}"; do
    # 定位 grp 分组内的 arch 块
    in_block=0; in_arch=0
    while IFS= read -r line; do
      case "${line}" in
        '['*']')
          g=${line#[}; g=${g%]}
          in_block=0; in_arch=0
          [ "${g}" = "${grp}" ] && in_block=1
          ;;
        *':')
          if [ "${in_block}" = "1" ]; then
            a=${line%:}
            if [ "${a}" = "${ARCH}" ]; then
              in_arch=1
            else
              in_arch=0
            fi
          fi
          ;;
        *)
          if [ "${in_block}" = "1" ] && [ "${in_arch}" = "1" ]; then
            # 跳过空行和注释（允许行首空白）
            if [[ "${line}" =~ ^[[:space:]]*$ ]] || [[ "${line}" =~ ^[[:space:]]*# ]]; then
              continue
            fi
            # 去除行首空白（pattern 保持干净）
            pattern="${line}"
            pattern="${pattern#"${pattern%%[![:space:]]*}"}"
            if [ -n "${LIT_FILTER_OUT}" ]; then
              LIT_FILTER_OUT="${LIT_FILTER_OUT}|${pattern}"
            else
              LIT_FILTER_OUT="${pattern}"
            fi
            FILTER_COUNT=$((FILTER_COUNT + 1))
          fi
          ;;
      esac
    done < "${FAILURES_CONF}"
  done

  if [ -n "${LIT_FILTER_OUT}" ]; then
    log_info "matched branch='${KEY}' arch='${ARCH}': ${FILTER_COUNT} filters"
    log_info "LIT_FILTER_OUT=${LIT_FILTER_OUT}"
    export LIT_FILTER_OUT=${LIT_FILTER_OUT}
  else
    # 诊断：区分"无匹配分支"与"分支存在但无当前 arch 用例"
    has_star=$(grep -c '^\[\*\]$' "${FAILURES_CONF}" 2>/dev/null)
    has_arch=$(awk -v arch="${ARCH}" '/^\[\*\]$/{f=1;next} /^\[/{f=0} f && $0 ~ ("^" arch ":"){print;exit}' "${FAILURES_CONF}" 2>/dev/null)
    if [ -z "${has_arch}" ]; then
      log_warn "no filters for branch='${KEY}' arch='${ARCH}': [*] section exists=${has_star} but has no '${ARCH}:' block"
    else
      log_warn "no filters for branch='${KEY}' arch='${ARCH}': group '${KEY}' + [*] have no non-comment pattern for '${ARCH}'"
    fi
    log_warn "tests will run without filtering"
  fi
fi


# ---- 4. PR 已通过缓存检测 -------------------------------------------------

log_step "skip-check: PR already passed?"

# 变量校验（缺失时报错退出，避免带空值进入构建）
: "${prid:?missing required env: prid}"
: "${branch:?missing required env: branch}"
: "${tbranch:?missing required env: tbranch}"
: "${committer:?missing required env: committer}"
: "${WORKSPACE:?missing required env: WORKSPACE}"

PR_ID=${prid}
SOURCE_BRANCH=${branch}
DEST_BRANCH=${tbranch}
AUTHOR=${committer}
PARALLEL_JOBS="16"

if [ "${ARCH}" = "aarch64" ]; then
  if [ "${DEST_BRANCH}" = "dev_17.0.6" ]; then
    PARALLEL_JOBS="32"
  fi
elif [ "${ARCH}" = "x86_64" ]; then
  PARALLEL_JOBS="32"
fi

# PR 缓存检测开关：默认开启；设 0 关闭（强制全量重测）
SKIP_IF_PASSED=${SKIP_IF_PASSED:-1}
if [ "${SKIP_IF_PASSED}" != "0" ] && [ "${SKIP_IF_PASSED}" != "1" ]; then
  log_warn "SKIP_IF_PASSED=${SKIP_IF_PASSED} invalid, fallback to 1 (enabled)"
  SKIP_IF_PASSED=1
fi
log_info "SKIP_IF_PASSED=${SKIP_IF_PASSED}"

# PR 状态仓库令牌：命令行参数优先，其次环境变量，最后 NONE
TOKEN="NONE"
if [ -n "$1" ]; then
  TOKEN="$1"
elif [ -n "${PR_STATUS_TOKEN:-}" ]; then
  TOKEN="${PR_STATUS_TOKEN}"
fi
if [ "${TOKEN}" = "NONE" ]; then
  log_warn "no PR status token provided; skip-check will be degraded (always full build)"
fi

cd "${WORKSPACE}/llvm-project" || exit 1
COMMIT_ID=$(git rev-parse HEAD)
log_info "PR_ID=${PR_ID} source=${SOURCE_BRANCH} dest=${DEST_BRANCH} commit=${COMMIT_ID}"

SKIP="0"
if [ "${SKIP_IF_PASSED}" = "1" ]; then
  STATUS_DIR="${WORKSPACE}/pr_status"
  JSON_FILE="${STATUS_DIR}/pr${PR_ID}.json"

  rm -rf "${STATUS_DIR}"
  if [ -z "${TOKEN}" ] || [ "${TOKEN}" = "NONE" ]; then
    log_warn "skip-check: no token; continue with full build"
  elif git clone --branch "${ARCH}" "https://eastb233:${TOKEN}@atomgit.com/eastb233/pr_status.git" "${STATUS_DIR}" >/dev/null 2>&1; then
    if [ -f "${JSON_FILE}" ]; then
      JSON_SOURCE_BRANCH=$(jq -r '.source_branch // empty' "${JSON_FILE}")
      JSON_DEST_BRANCH=$(jq -r '.dest_branch // empty' "${JSON_FILE}")
      JSON_AUTHOR=$(jq -r '.author // empty' "${JSON_FILE}")
      JSON_COMMIT_ID=$(jq -r '.commit_id // empty' "${JSON_FILE}")
      JSON_STATUS=$(jq -r '.status // empty' "${JSON_FILE}")

      if [ -n "${JSON_SOURCE_BRANCH}${JSON_DEST_BRANCH}${JSON_AUTHOR}${JSON_COMMIT_ID}${JSON_STATUS}" ] && \
         [ "${JSON_SOURCE_BRANCH}" = "${SOURCE_BRANCH}" ] && \
         [ "${JSON_DEST_BRANCH}" = "${DEST_BRANCH}" ] && \
         [ "${JSON_AUTHOR}" = "${AUTHOR}" ] && \
         [ "${JSON_COMMIT_ID}" = "${COMMIT_ID}" ] && \
         [ "${JSON_STATUS}" = "success" ]; then
        SKIP="1"
      else
        log_info "skip-check: previous record for PR${PR_ID} mismatch or not success; continue with full build"
      fi
    else
      log_info "skip-check: no previous record for PR${PR_ID}; continue with full build"
    fi
  else
    log_warn "skip-check: failed to clone pr_status repo; continue with full build"
  fi
else
  log_info "skip-check: SKIP_IF_PASSED=0, forced full build"
fi

if [ "${SKIP}" = "1" ]; then
  log_info "PR already passed and unchanged; exit 0"
  exit 0
fi

# ---- 5. 执行构建 ---------------------------------------------------------

log_step "building with build.sh (jobs=${PARALLEL_JOBS})"

STATUS="fail"
RESULT=0

bash build.sh -v -r -t -b relwithdebinfo -E -i \
  -I "install-${ARCH}-for-pr-${PR_ID}" -j"${PARALLEL_JOBS}"
RESULT=$?
if [ "${RESULT}" -eq 0 ]; then
  STATUS="success"
  log_step "build succeeded"
else
  log_error "build failed with exit code ${RESULT}"
fi

# ---- 6. 结果回写 ---------------------------------------------------------

log_step "recording result (status=${STATUS})"

if [ -d "${STATUS_DIR}" ] && { [ -n "${TOKEN}" ] && [ "${TOKEN}" != "NONE" ]; }; then
  COMMIT_FILE="${STATUS_DIR}/pr${PR_ID}.json"
  cat > "${COMMIT_FILE}" <<EOF
{
  "pr_id": "${PR_ID}",
  "source_branch": "${SOURCE_BRANCH}",
  "dest_branch": "${DEST_BRANCH}",
  "author": "${AUTHOR}",
  "commit_id": "${COMMIT_ID}",
  "status": "${STATUS}",
  "timestamp": "$(date -Iseconds)"
}
EOF

  git -C "${STATUS_DIR}" config user.name eastb233
  git -C "${STATUS_DIR}" config user.email xiezhiheng@huawei.com
  git -C "${STATUS_DIR}" add -A
  if git -C "${STATUS_DIR}" diff --cached --quiet; then
    log_info "pr_status: no change to push"
  else
    git -C "${STATUS_DIR}" commit -m "CI adds PR${PR_ID} json" >/dev/null 2>&1 && \
      git -C "${STATUS_DIR}" push origin "${ARCH}" >/dev/null 2>&1 && \
      log_info "pr_status: pushed PR${PR_ID} result" || \
      log_warn "pr_status: failed to push result (recorded locally)"
  fi
else
  log_warn "pr_status: skip writing result (repo or token unavailable)"
fi

log_step "done: status=${STATUS}"
exit "${RESULT}"
