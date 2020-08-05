import os
import subprocess
import re

TARGET_DIR = './StageOne'
SRC_DIR = f'{TARGET_DIR}/src'
LOG_DIR = f'{TARGET_DIR}/test'

def main():
    src_list= os.listdir(SRC_DIR)
    for src in src_list:
        name = src.split('.')[0]
        if os.path.exists(f"{LOG_DIR}/{name}.log"):
            change = input(f'Do you want to change name {name}?: ')
            if change != '':
                new_name = input('give new name: ')
                subprocess.run(['mv', f'{SRC_DIR}/{name}.c', f'{SRC_DIR}/{new_name}.c'])

                with open(f'{LOG_DIR}/{name}.log', 'r+') as f:
                    log_content = f.readlines()
                for i in range(len(log_content)):
                    log_content[i].replace(name, new_name)
                with open(f'{LOG_DIR}/{new_name}.log', 'w+') as f:
                    f.write(''.join(log_content))

                subprocess.run(['rm', f'{LOG_DIR}/{name}.log'])
    return

def rename_log():
    log_list = os.listdir(LOG_DIR)
    for log in log_list:
        name = log.split('.')[0]
        print(name)
        with open(f'{LOG_DIR}/{name}.log', 'r+') as f:
            log_content = f.readlines()
        for i in range(len(log_content)):
            log_content[i] = log_content[i].replace(name, name + '.c')
            print(log_content[i])
        with open(f'{LOG_DIR}/{name}.log', 'w+') as f:
            f.write(''.join(log_content))

    return


if __name__ == "__main__":
    rename_log()
    # main()
