import time

n = 10_000_000
sum_value = 0

start = time.perf_counter()
for _ in range(n):
    sum_value += 1
end = time.perf_counter()

print(sum_value)
print(f"elapsed_ms={(end - start) * 1000:.3f}")
