#!/usr/bin/env python3
import re
import os
import sys
import subprocess
from itertools import permutations

# function to read options from the instList.txt file
def read_bisheng_options(file_path):
    with open(file_path, 'r') as file:
        options = [line.strip() for line in file.readlines()]
    return options

def extract_opt_options(file_path, bisheng_options):
    with open(file_path, 'r') as file:
        content = file.read()
    tool_info_list = []
    # Define a pattern to match a single RUN line
    run_pattern = r'; RUN:(.*?)(?=\n; RUN:|$)'
    run_matches = re.finditer(run_pattern, content, re.DOTALL)
    for run_match in run_matches:
        run_content = run_match.group(1).strip()
        # Find the index of the first '|' character
        first_pipe_index = run_content.find('|')

        if first_pipe_index != -1:
        # Split run_content at the first '|' character
            run_content = run_content[:first_pipe_index]

        # Extract the tool name from the RUN line
        tool_name_match = re.search(r'(\w+)', run_content)
        if tool_name_match:
            tool_name = tool_name_match.group(1)
        else:
            continue  # Skip if tool name is not found
        
        # Extract options from the RUN line
        tool_option_match = re.search(r'\b' + re.escape(tool_name) + r'\b\s+(.*)', run_content)
        if tool_option_match:
            tool_options = tool_option_match.group(1).strip().split()
        else:
            tool_options = []

        # Replace %s in each option with the input file path
        current_file_path = os.path.abspath(file_path)
        tool_options_with_path = [opt.replace('%s', current_file_path) for opt in tool_options]

        # Match options in the IR file with options in bisheng_options
        need_test_option = [opt for opt in tool_options_with_path if any(bisheng_opt in opt for bisheng_opt in bisheng_options)]

        # Store options other than those in need_test_option
        other_options = [opt for opt in tool_options_with_path if opt not in need_test_option]

        tool_info_list.append((tool_name, other_options, need_test_option))

    return tool_info_list


def run_tool(input_file, tool_options, tool_name):
    tool_command = f"{tool_name} {' '.join(tool_options)} {input_file} -o {input_file}.bc"
    try:
        subprocess.run(tool_command, shell=True, check=True)
        return f"{input_file}.bc"
    except subprocess.CalledProcessError:
        print(f"Failed to run the {tool_name} tool")
        return None

def run_alive_tv(input_file, bc_file):
    alive_tv_command = f"alive-tv {input_file} {bc_file}"
    try:
        result = subprocess.run(alive_tv_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if "Transformation seems to be correct!" in result.stdout and "ERROR" not in result.stdout:
            return True
        else:
            return False
    except subprocess.CalledProcessError:
        print("Failed to run the alive-tv tool")
        return False

def test_optionX_combinations(input_file, need_test_option, other_options, tool_name):

    unique_optionX_combinations = set()  
    for r in range(1, len(need_test_option) + 1):
        for combination in permutations(need_test_option, r):
            sorted_combination = tuple(sorted(combination))  
            if sorted_combination not in unique_optionX_combinations:
                unique_optionX_combinations.add(sorted_combination)
                
                all_tool_options = list(combination) + other_options
                bc_file = run_tool(input_file, all_tool_options,tool_name)
                if bc_file:
                    if run_alive_tv(input_file, bc_file):
                        print(f"Successful option combination for '{tool_name}': {combination}")
                    else:
                        print(f"Error: Failed option combination for '{tool_name}': {combination}")
                else:
                    print(f"{tool_name} couldn't run this Option combination : {combination}")
                if bc_file:
                    os.remove(bc_file)

if len(sys.argv) != 2:
    print("Please provide the path to an LLVM IR file as a command-line argument")
else:
    llvm_ir_path = sys.argv[1]
    bisheng_option_file = "instList.txt"
    bisheng_options = read_bisheng_options(bisheng_option_file)

    tool_info_list = extract_opt_options(llvm_ir_path, bisheng_options)
    
    for tool_info in tool_info_list:
        tool_name, other_options, need_test_option = tool_info
        if need_test_option:
            test_optionX_combinations(llvm_ir_path, need_test_option, other_options, tool_name)
