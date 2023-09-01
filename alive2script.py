import re
import os
import sys
import subprocess
from itertools import permutations

def extract_opt_options(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    pattern = r'; RUN:.*\bopt\b ([^\|]*)'
    matches = re.findall(pattern, content)
    if matches:
        opt_options = []
        for match in matches:
            options = match.strip().split()
            opt_options.extend(options)
        
        # Replace %s in each option with the input file path
        current_file_path = os.path.abspath(file_path)
        opt_options_with_path = [opt.replace('%s', current_file_path) for opt in opt_options]
        
        optionX_options = [opt for opt in opt_options_with_path if re.match(r'-option', opt)]
        print("\n所有形式为 -optionX= 的属于 opt 工具的选项:")
        for option in optionX_options:
            print(option)

        other_options = [opt for opt in opt_options_with_path if opt not in optionX_options]
        print("\n所有其他属于 opt 工具的选项:")
        for option in other_options:
            print(option)
        return optionX_options, other_options
    else:
        print("未找到属于 opt 工具的选项")
        return []

def run_opt(input_file, opt_options):
    opt_command = f"opt {' '.join(opt_options)} {input_file} -o {input_file}.bc"
    try:
        subprocess.run(opt_command, shell=True, check=True)
        return f"{input_file}.bc"
    except subprocess.CalledProcessError:
        print("运行 opt 工具失败")
        return None

def run_alive_tv(input_file, bc_file):
    alive_tv_command = f"alive-tv {input_file} {bc_file}"
    try:
        result = subprocess.run(alive_tv_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if "Transformation seems to be correct!" in result.stdout:
            return True
        else:
            return False
    except subprocess.CalledProcessError:
        print("运行 alive-tv 工具失败")
        return False

def test_optionX_combinations(input_file, optionX_options, other_opt_options):

    unique_optionX_combinations = set()  
    
    for r in range(1, len(optionX_options) + 1):
        for combination in permutations(optionX_options, r):
            sorted_combination = tuple(sorted(combination))  
            if sorted_combination not in unique_optionX_combinations:
                unique_optionX_combinations.add(sorted_combination)
                
                all_opt_options = list(combination) + other_opt_options
                bc_file = run_opt(input_file, all_opt_options)
                if bc_file:
                    if run_alive_tv(input_file, bc_file):
                        print(f"成功的选项组合: {combination}")
                        success_found = True
                    else:
                        print(f"失败的选项组合: {combination}")
                        failure_found = True
                else:
                    print(f"选项组合无法运行 opt 工具: {combination}")
                    failure_found = True
                
                
                if bc_file:
                    os.remove(bc_file)
    

if len(sys.argv) != 2:
    print("请提供一个 LLVM IR 文件的路径作为命令行参数")
else:
    llvm_ir_path = sys.argv[1]
    optionX_options, other_opt_options = extract_opt_options(llvm_ir_path)
    if optionX_options:
        test_optionX_combinations(llvm_ir_path, optionX_options, other_opt_options)
