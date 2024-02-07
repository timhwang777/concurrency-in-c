import os
import random
import string
import re

def generate_test_case(index, filename, num_chars):
    # Generate random data
    data = ''.join(random.choice(string.ascii_letters) for _ in range(num_chars))

    # Count the number of test cases right now in the current directory
    test_case_count = 0
    for file in os.listdir("."):
        # File name have the format test0, test1, test2, ...
        # print(f"The file: {file}")
        match = re.match(f"{filename}(\d+)", file)
        if match:
            # Print the file name
            # print(f"Matched file: {file}")
            test_case_count = max(test_case_count, int(match.group(1)) + 1)
    
    # Write the data file
    with open(f"{filename}{test_case_count}", "w") as f:
        f.write(data)

    # Write the expected compressed file
    # with open(filename + ".z", "w") as f:
    #     for char, count in rle_encode(data):
    #         f.write(f"{count}{char}")

# Generate 10 test cases
# Change the range to generate more test cases
for i in range(2):
    generate_test_case(i, "test", num_chars=500)