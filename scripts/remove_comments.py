import os
import glob
import sys

def strip_comments(text):
    result = []
    in_string = False
    in_char = False
    in_multiline_comment = False
    escape = False
    i = 0
    n = len(text)
    
    while i < n:
        c = text[i]
        
        if not in_multiline_comment:
            if escape:
                result.append(c)
                escape = False
                i += 1
                continue
                
            if c == '\\':
                escape = True
                result.append(c)
                i += 1
                continue
                
            if c == '"' and not in_char:
                # Basic handling, won't cover R"()" but good enough for 99%
                in_string = not in_string
                result.append(c)
                i += 1
                continue
                
            if c == "'" and not in_string:
                in_char = not in_char
                result.append(c)
                i += 1
                continue

        if not in_string and not in_char:
            if not in_multiline_comment:
                if c == '/' and i + 1 < n and text[i+1] == '/':
                    # Single line comment
                    # Skip until newline
                    while i < n and text[i] != '\n' and text[i] != '\r':
                        i += 1
                    continue
                elif c == '/' and i + 1 < n and text[i+1] == '*':
                    in_multiline_comment = True
                    i += 2
                    continue
            else:
                if c == '*' and i + 1 < n and text[i+1] == '/':
                    in_multiline_comment = False
                    i += 2
                    continue
                else:
                    if c == '\n' or c == '\r':
                        result.append(c)
                    i += 1
                    continue
                    
        result.append(c)
        i += 1
        
    return "".join(result)

def process_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8', newline='') as f:
            content = f.read()
            
        new_content = strip_comments(content)
        
        # Clean trailing whitespaces on each line, keeping original newlines
        lines = new_content.splitlines(keepends=True)
        cleaned_lines = []
        for line in lines:
            if line.endswith('\r\n'):
                cleaned_lines.append(line[:-2].rstrip() + '\r\n')
            elif line.endswith('\n'):
                cleaned_lines.append(line[:-1].rstrip() + '\n')
            else:
                cleaned_lines.append(line.rstrip())
                
        final_output = "".join(cleaned_lines)
        
        if content != final_output:
            with open(filepath, 'w', encoding='utf-8', newline='') as f:
                f.write(final_output)
            return True
        return False
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False

def main():
    target_dirs = ['src', 'include']
    extensions = ['*.cpp', '*.hpp', '*.h', '*.c']
    
    processed = 0
    modified = 0
    
    for d in target_dirs:
        for ext in extensions:
            for filepath in glob.glob(f'{d}/**/{ext}', recursive=True):
                if process_file(filepath):
                    modified += 1
                processed += 1
                
    print(f"Scanned {processed} files, removed comments from {modified} files.")

if __name__ == '__main__':
    main()
