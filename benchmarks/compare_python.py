import time

N = 10_000_000

# Warmup
acc = 0
for i in range(1000):
    acc += i

start = time.perf_counter()
acc = 0
for i in range(N):
    acc += i
end = time.perf_counter()

elapsed = end - start
ns_per_op = (elapsed / N) * 1e9

print(acc)
print(f"elapsed_s={elapsed:.6f}")
print(f"ns_per_op={ns_per_op:.2f}")
