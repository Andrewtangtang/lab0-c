import subprocess
import re
import random
from itertools import permutations
import random
import os
import matplotlib.pyplot as plt
import numpy as np

# 測試 shuffle 次數
test_count = 100000
input = "new\nit 1\nit 2\nit 3\n"
for i in range(test_count):
    input += "shuffle\n"
    if i % 100000 == 0:
        print(f"concatenate {i} times")
input += "free\nquit\n"

# 保存當前目錄
original_dir = os.getcwd()

try:
    os.chdir('..')  # change to the main directory
    
    
    
    # get the shuffle result from stdout
    command='./qtest -v 3'  # use ./qtest instead of ../qtest
    clist = command.split()
    completedProcess = subprocess.run(clist, capture_output=True, text=True, input=input)

    # 檢查程式是否正確執行
    if completedProcess.returncode != 0:
        print("Error: qtest execution failed")
        print("STDERR:", completedProcess.stderr)
        exit(1)

    s = completedProcess.stdout

    # find the initial state
    init_pos = s.find("l = [1 2 3]")
    if init_pos != -1:
        # 從初始狀態之後開始搜尋
        s = s[init_pos + len("l = [1 2 3]"):]

    # 使用更精確的正則表達式
    Regex = re.compile(r'l = \[(\d+) (\d+) (\d+)\]')
    matches = Regex.finditer(s)
    results = []

    for match in matches:
        # get the numbers
        numbers = [match.group(1), match.group(2), match.group(3)]
        results.append(numbers)

    print(f"\nfound {len(results)} shuffle results")
    print("example of the first 5 results:")
    for i in range(min(5, len(results))):
        print(results[i])
    print("=================")

    def permute(nums):
        nums=list(permutations(nums,len(nums)))
        return nums

    def chiSquared(observation, expectation):
        return ((observation - expectation) ** 2) / expectation 

    # 找出全部的排序可能
    counterSet = {}
    shuffle_array = ['1', '2', '3']
    s = permute(shuffle_array)

    # 初始化 counterSet
    for i in range(len(s)):
        w = ''.join(s[i])
        counterSet[w] = 0

    # calculate the number of each shuffle result
    for nums in results:
        permutation = ''.join(nums)
        counterSet[permutation] += 1

    # verify the number of results
    total_count = sum(counterSet.values())
    print(f"\ntotal count of results: {total_count}")
    if total_count != test_count:
        print(f"Warning: the number of results ({total_count}) is not equal to the test count ({test_count})")
        
    # calculate the chiSquare sum
    expectation = test_count // len(s)
    c = counterSet.values()
    chiSquaredSum = 0
    
    # Calculate and print individual chi-squared values
    print("\nChi-squared values for each permutation:")
    print("Permutation | Observation | Expected | Chi-squared")
    print("-" * 50)
    
    individual_chi_squares = {}
    for perm, count in counterSet.items():
        chi_square = chiSquared(count, expectation)
        individual_chi_squares[perm] = chi_square
        chiSquaredSum += chi_square
        print(f"{perm:10} | {count:10d} | {expectation:8d} | {chi_square:10.4f}")
    
    print("-" * 50)
    print(f"Total chi-square sum: {chiSquaredSum:.4f}")
    print(f"Degrees of freedom: {len(s) - 1}")
    print("Expectation: ", expectation)
    print("Chi-square sum: ", chiSquaredSum)

    # draw the histogram
    try:
        # prepare the data
        permutations = list(counterSet.keys())
        counts = list(counterSet.values())
        
        # create two subplots
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 12))
        
        # First subplot: Frequency distribution
        x = np.arange(len(counts))
        ax1.bar(x, counts)
        ax1.set_xticks(x)
        ax1.set_xticklabels(permutations, rotation=45)
        ax1.set_xlabel('Permutations')
        ax1.set_ylabel('Frequency')
        ax1.set_title('Shuffle Distribution')
        ax1.axhline(y=expectation, color='r', linestyle='--', label='Expected Value')
        ax1.legend()
        
        # Second subplot: Chi-squared values
        chi_squares = [individual_chi_squares[p] for p in permutations]
        ax2.bar(x, chi_squares)
        ax2.set_xticks(x)
        ax2.set_xticklabels(permutations, rotation=45)
        ax2.set_xlabel('Permutations')
        ax2.set_ylabel('Chi-squared Value')
        ax2.set_title('Chi-squared Distribution')
        
        # add the statistical information
        plt.figtext(0.02, 0.98, f'Total Count: {total_count}\n'
                                f'Expected Value: {expectation}\n'
                                f'Total Chi-square: {chiSquaredSum:.4f}',
                   verticalalignment='top',
                   bbox=dict(boxstyle='round', facecolor='white', alpha=0.8))
        
        # adjust the layout and show the chart
        plt.tight_layout()
        
        # save the chart in the tests folder
        save_path = os.path.join(original_dir, 'shuffle_distribution.png')
        plt.savefig(save_path)  # save the chart
        print(f"\nChart saved as: {save_path}")
        plt.show()
    except Exception as e:
        print(f"\nError drawing chart: {e}")
        print("Please make sure matplotlib and numpy are installed:")
        print("pip install matplotlib numpy")

finally:
    # ensure the last directory is the original directory
    os.chdir(original_dir)
