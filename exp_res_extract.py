import re
import os
import re
import matplotlib.pyplot as plt


PARAMS='1536'
MEM_UTIL = []
def convert_to_numbers(value: str):
    #TODO: add comma logic 
    return float(value) if '.' in value else int(value)


def mapping_key_names(output: str):
    if output == "total page access":
        return "page_access"
    if output == "number of pagefaults":
        return "pagefault_num"
    if output == "number of swap":
        return "swap_num"
    if output == "first swap memory utilization":
        return "mem_util"
    if output == "average age of swapped out pages":
        return "avg_age"

def mapping_label_to_title(label: str):
    if label == "pagefault_num":
        return "Number of Page Fault"
    if label == "swap_num":
        return "Number of Swap"
    if label == "mem_util":
        return "Memory Utilization"
    if label == "avg_age":
        return "Average Age of Swapped-out Pages"

def convert_file_name_to_legend(file_name: str):
    if "uni-dyn-ind" in file_name:
        return "uni-hash + dynamic function"
    if "uni-dyn-xor" in file_name:
        return "uni-hash + XOR method"
    if "uni-static" in file_name:
        return "uni-hash + static function"
    if "ice" in file_name:
        return "iceberg"
    
def extract_result_from_file(file_path: str):
    exp_stat = {}
    SKIP = False
    with open(file_path, 'r') as file:
        current_mem_access = None
        for line in file:
            if ":" not in line:
                continue
            if "total memory access" in line:
                current_mem_access = int(line.split(":")[-1].strip().replace(",",""))
                exp_stat[current_mem_access] = {}
            if "first swap memory utilization" in line and not SKIP:
                _, value = map(str.strip, line.split(':'))
                MEM_UTIL.append(convert_to_numbers(value.replace(",","")))
                SKIP = True
            elif current_mem_access is not None:
                key, value = map(str.strip, line.split(':'))
                exp_stat[current_mem_access][mapping_key_names(key)] = convert_to_numbers(value.replace(",",""))
    return exp_stat

def convert_to_int(values):
    return [int(val) for val in values]

def extract_messages_from_folder(folder_path: str):
    results_dict = {}
    files = os.listdir(folder_path)
    files = [file for file in files if PARAMS in file]
    for file_name in files:
        file_path = os.path.join(folder_path, file_name)

        if os.path.isfile(file_path) and ".out" in file_path:
            result = extract_result_from_file(file_path)
            results_dict[file_name] = result

    return results_dict

def draw_experiment_comparing_results(attribute: str, results_dict: dict, folder_path: str):
    if attribute == "mem_util":
        draw_mem_util(results_dict, folder_path)
        return
    
    if attribute == "avg_age":
        plt.figure()
        for file_name, single_exp_result in results_dict.items():
            x_range = [key for key in single_exp_result.keys() if attribute in single_exp_result[key]]
            xx_range = [single_exp_result[key]["swap_num"] for key in x_range if single_exp_result[key]["swap_num"]]
            y_range = [single_exp_result[key][attribute] for key in x_range if single_exp_result[key]["swap_num"]]
            if "uni-dyn-xor" in file_name:
                plt.plot(xx_range, y_range, label=convert_file_name_to_legend(file_name), linestyle='--', marker='x',markersize=5, color='red')
            elif "uni-dyn-ind" in file_name:
                plt.plot(xx_range, y_range, label=convert_file_name_to_legend(file_name), linestyle='-', marker='o',markersize=1, color='blue')
                
            elif 'ice' in file_name:   
                plt.plot(xx_range, y_range, label=convert_file_name_to_legend(file_name), color='green') #TODO: process file_name to readable format
            else:
                plt.plot(xx_range, y_range, label=convert_file_name_to_legend(file_name), color='yellow') #TODO: process file_name to readable format

        plt.xlabel('Number of Swaps')
        plt.ylabel(mapping_label_to_title(attribute))
        plt.legend() 
        plt.grid(True)
        plt.savefig(folder_path + "/{}_".format(PARAMS) + attribute +"_out.jpg")    
        return
    
    plt.figure()
    for file_name, single_exp_result in results_dict.items():
        x_range = [key for key in single_exp_result.keys() if attribute in single_exp_result[key]]
        y_range = [single_exp_result[key][attribute] for key in x_range]
        if "uni-dyn-xor" in file_name:
            plt.plot(x_range, y_range, label=convert_file_name_to_legend(file_name), linestyle='--', marker='x',markersize=5, color='blue')
        elif "uni-dyn-ind" in file_name:
            plt.plot(x_range, y_range, label=convert_file_name_to_legend(file_name), linestyle='-', marker='o',markersize=2, color='red')
             
        elif "ice" in file_name:
            plt.plot(x_range, y_range, label=convert_file_name_to_legend(file_name), color='green') #TODO: process file_name to readable format
        else:
            plt.plot(x_range, y_range, label=convert_file_name_to_legend(file_name), color='yellow') #TODO: process file_name to readable format

    plt.xlabel('Memory Access')
    plt.ylabel(mapping_label_to_title(attribute))
    # plt.title("BTree" + mapping_label_to_title(attribute)) #TODO: add experiment param and benchmark 
    plt.legend()
    plt.grid(True)
    
    plt.savefig(folder_path + "/{}_".format(PARAMS) + attribute +"_out.jpg")    

def draw_mem_util(results_dict: dict, folder_path: str):

    file_names = [convert_file_name_to_legend(file_name) for file_name in results_dict.keys()]
    
    plt.figure()
    import numpy as np
    bar_colors = plt.cm.viridis(np.linspace(0, 1, len(file_names)))
    plt.bar(file_names, MEM_UTIL, color=bar_colors, edgecolor='black')

    plt.xlabel('Hashing Schemes')
    plt.ylabel('Memory Utilization at first swap')
    plt.grid(True)

    plt.savefig(folder_path + "/{}_".format(PARAMS) + "mem_util_out.jpg")    

if __name__ == "__main__":
    folder_path = '/Users/liqilin/Desktop/extract_contents/exp_gups'
    output_path = folder_path + "/output"
    os.makedirs(output_path, exist_ok=True)
    results = extract_messages_from_folder(folder_path)
    labels = ["pagefault_num", "swap_num", "avg_age", "mem_util"]
    for label in labels:
        draw_experiment_comparing_results(label, results, output_path)
    print()