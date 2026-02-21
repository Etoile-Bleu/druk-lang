import sys
import re

def filter_errors(input_stream):
    # Match lines containing "error:" (case-insensitive) but allow for context
    error_pattern = re.compile(r'.*error:.*', re.IGNORECASE)
    warning_pattern = re.compile(r'.*warning:.*', re.IGNORECASE)
    
    errors = []
    capture_context = 0
    
    for line in input_stream:
        if warning_pattern.match(line):
            capture_context = 0
            continue
            
        if error_pattern.match(line):
            errors.append(line)
            capture_context = 3
        elif capture_context > 0:
            errors.append(line)
            capture_context -= 1
                
    return errors

if __name__ == "__main__":
    lines = sys.stdin.readlines()
    filtered = filter_errors(lines)
    
    with open("compilation_errors.log", "w") as f:
        f.writelines(filtered)
    
    for line in filtered:
        sys.stdout.write(line)
    
    if filtered:
        sys.exit(1)
    sys.exit(0)
