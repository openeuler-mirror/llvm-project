#!/bin/sh
pip install pyinstaller
rm -rf build/
rm -rf dist/
rm -f bishengai.spec
TARGET="bishengai"
SOURCE="src/compiler_driver.py"
pyinstaller --onefile $SOURCE --name $TARGET
export CC=`pwd`/dist/$TARGET
export CXX=$CC
export LLM_DEVELOPMENT=0
# export COMPILER_CHOICE="clang++" # "clang" for c; "clang++"" for c++
export LLM_DEBUG=1
export AUTO_ACCEPT=1 # not export-interactive window; export-automatically change


# 1：流式 0：非流式
export ENABLE_STREAM=0

# openai
export LLM_MODEL_TYPE=openai
export LLM_API_TOKEN=xxxxxxxxxxxxxxxxxxx
# export LLM_MODEL=deepseek-ai/DeepSeek-R1-Distill-Qwen-14B
# export LLM_MODEL=Qwen/Qwen3-14B
export LLM_MODEL=Qwen/Qwen3-8B
export LLM_URL=https://api.siliconflow.cn/v1/chat/completions

# 本地ollama
# export LLM_MODEL_TYPE=local_ollama
# export LLM_URL=http://localhost:11434/api/generate
# export LLM_MODEL=deepseek-r1:1.5b


# export LLM_MODEL_TYPE=openai
# export LLM_MODEL=/data/zrf/models/DeepSeek-R1-Distill-Qwen-14B-F16.gguf
# export LLM_URL=http://127.0.0.1:8080/v1/chat/completions