#!/usr/bin/env python3

import json
import os
from dataclasses import dataclass, field
from typing import List

@dataclass
class TestResult:
    test_name: str
    status: str
    process_outputs: List[dict] = field(default_factory=list)


def extract_top_level_directory(test_name):
    # Assuming the test name is a file path, extract the top-level directory
    return os.path.dirname(test_name).split("/")[1]


def parse_log_file(file_path, max_entries=100):
    with open(file_path, 'r') as file:
        current_test = None
        test_results = []
        process_outputs = []
        flagged_tests = set()
        entry_count = 0

        for line in file:
            if entry_count >= max_entries:
                break

            try:
                log_entry = json.loads(line)

                if log_entry.get("action") == "test_start":
                    current_test = log_entry.get("test")
                    process_outputs = []

                elif log_entry.get("action") == "test_end":
                    process_test_end(log_entry, test_results, process_outputs)
                    entry_count += 1

                elif log_entry.get("action") == "process_output" and current_test is not None:
                    process_outputs.append(log_entry)
                    flag_test(log_entry, current_test, flagged_tests)

            except json.JSONDecodeError as e:
                print(f"Error parsing JSON: {e}")

        return test_results, flagged_tests


def process_test_end(log_entry, test_results, process_outputs):
    test_status = log_entry.get("status", "")
    test_name = log_entry.get("test", "")
    test_result = TestResult(test_name=test_name, status=test_status, process_outputs=process_outputs)
    test_results.append(test_result)


def flag_test(log_entry, current_test, flagged_tests):
    data = log_entry.get("data", "")
    if 'VERIFICATION FAILED' in data:
        flagged_tests.add(current_test)


if __name__ == "__main__":
    log_file_path = "wpt-css-bigger.log"  # Replace with the actual path to your log file
    test_results, flagged_tests = parse_log_file(log_file_path, max_entries=1000000000)

    # Aggregate test results by top-level directory
    aggregated_results = {}
    for test_result in test_results:
        top_level_directory = extract_top_level_directory(test_result.test_name)
        status = test_result.status
        aggregated_results.setdefault(top_level_directory, {'PASS': 0, 'TIMEOUT': 0, 'FAIL': 0, 'SKIP': 0, 'ERROR': 0, 'OK': 0, 'CRASH': 0})[status] += 1


    # Sort aggregated results by the number of "TIMEOUT" statuses
    sorted_results = sorted(aggregated_results.items(), key=lambda x: x[1].get('TIMEOUT', 0), reverse=True)

    # Print sorted aggregated test results
    print("\nSorted Aggregated Test Results by Number of TIMEOUT:")
    for directory, results in sorted_results:
        print(f"{directory}: {results}")
    
    # Print flagged tests
    print(f"\nFlagged Tests: {len(flagged_tests)}")
    aggregated_flags = {}
    for flagged_test in flagged_tests:
        top_level_directory = extract_top_level_directory(flagged_test)
        aggregated_flags.setdefault(top_level_directory, []).append(flagged_test)

    test_dict = { test_result.test_name: test_result for test_result in test_results }

    crashy_tests = sorted(aggregated_flags.items(), key=lambda x: len(x[1]), reverse=True)

    for directory, tests in crashy_tests:
        print(f"{directory}: {len(tests)}")

        if directory == 'css':
            with open('css.txt', 'w') as f:
                for test in tests:
                    f.write(test_dict[test].test_name + "\n")
                    for output in test_dict[test].process_outputs:
                        f.write(output['data'])
                        f.write("\n")
                    f.write("\n\n")
