import os
import subprocess
import sys
import argparse
from typing import List, Tuple

def run_command(command: List[str], cwd: str = ".") -> Tuple[int, str, str]:
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
        encoding="utf-8",
        cwd=cwd
    )
    stdout, stderr = process.communicate()
    return process.returncode, stdout.strip(), stderr.strip()

import time

def run_test(case_path: str, compiler_path: str, mode: str) -> bool:
    name = os.path.basename(case_path)
    out_path = case_path.replace(".druk", ".out")
    err_path = case_path.replace(".druk", ".err")
    
    expected_output = None
    expected_errors = None
    
    if os.path.exists(out_path):
        with open(out_path, "r", encoding="utf-8") as f:
            expected_output = f.read().strip()
    
    if os.path.exists(err_path):
        with open(err_path, "r", encoding="utf-8") as f:
            expected_errors = [line.strip() for line in f.readlines() if line.strip()]

    if expected_output is None and expected_errors is None:
        print(f"[SKIP] {name} (No .out or .err file found)")
        return True

    compile_time = 0.0
    exec_time1 = 0.0
    exec_time2 = 0.0
    total_time = 0.0

    if mode == "jit":
        start = time.perf_counter()
        rc, stdout, stderr = run_command([compiler_path, case_path])
        total_time = time.perf_counter() - start
        actual_output = stdout
    elif mode == "aot":
        exe_path = case_path.replace(".druk", ".exe")
        # Compile
        start_c = time.perf_counter()
        rc, stdout, stderr = run_command([compiler_path, "compile", case_path, "-o", exe_path])
        compile_time = time.perf_counter() - start_c
        
        if expected_errors:
            actual_output = "" # Error expected during compilation
        else:
            if rc != 0:
                print(f"[FAIL] {name} (AOT Compilation failed)")
                print(stderr)
                return False
            
            # Run 1 (Cold)
            start_e1 = time.perf_counter()
            rc, stdout, stderr = run_command([exe_path])
            exec_time1 = time.perf_counter() - start_e1
            actual_output = stdout

            # Run 2 (Warm)
            start_e2 = time.perf_counter()
            run_command([exe_path])
            exec_time2 = time.perf_counter() - start_e2
            
            # Cleanup
            if os.path.exists(exe_path): os.remove(exe_path)
            obj_path = exe_path.replace(".exe", ".obj")
            if os.path.exists(obj_path): os.remove(obj_path)
    else:
        print(f"Unknown mode: {mode}")
        return False

    time_str = ""
    if mode == "jit":
        time_str = f"({total_time:.3f}s total)"
    else:
        time_str = f"({compile_time:.3f}s compile, {exec_time1:.3f}s cold, {exec_time2:.3f}s warm)"

    status = "PASS"
    result_bool = True

    if expected_errors:
        if rc == 0:
            status = "FAIL"
            result_bool = False
            msg = "Expected error but exited with 0"
        else:
            missing_errors = [e for e in expected_errors if e not in stderr]
            if not missing_errors:
                status = "PASS"
                result_bool = True
            else:
                status = "FAIL"
                result_bool = False
                msg = f"Missing expected error messages. Expected: {expected_errors}"
    elif actual_output != expected_output:
        status = "FAIL"
        result_bool = False
        msg = f"Output mismatch. Expected: {repr(expected_output)}, Actual: {repr(actual_output)}"
    
    print(f"[{status}] {name} ({mode.upper()}) {time_str}")
    if not result_bool and status == "FAIL":
        if 'msg' in locals(): print(f"  {msg}")
        if stderr and not expected_errors: print(f"  Stderr: {stderr}")

    return result_bool

def main():
    if sys.platform == "win32":
        import io
        if isinstance(sys.stdout, io.TextIOWrapper):
            sys.stdout.reconfigure(encoding='utf-8')
        if isinstance(sys.stderr, io.TextIOWrapper):
            sys.stderr.reconfigure(encoding='utf-8')
            
    parser = argparse.ArgumentParser(description="Druk E2E Test Runner")
    parser.add_argument("--compiler", default="build/Release/druk.exe", help="Path to druk compiler")
    parser.add_argument("--cases", default="tests/e2e/cases", help="Directory containing test cases")
    args = parser.parse_args()

    if not os.path.exists(args.compiler):
        print(f"Compiler not found at {args.compiler}. Build it first!")
        sys.exit(1)

    cases = [os.path.join(args.cases, f) for f in os.listdir(args.cases) if f.endswith(".druk")]
    cases.sort()

    if not cases:
        print("No test cases found.")
        return

    failed = 0
    print(f"Running {len(cases)} tests in JIT and AOT modes...\n")

    for case in cases:
        # Run JIT
        if not run_test(case, args.compiler, "jit"):
            failed += 1
        # Run AOT
        if not run_test(case, args.compiler, "aot"):
            failed += 1

    print(f"\nTotal failures: {failed}")
    if failed > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
