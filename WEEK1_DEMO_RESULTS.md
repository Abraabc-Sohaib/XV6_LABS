## 1. Setup ✅

- Cloned xv6-riscv from MIT
- Created GP branch
- Modified Makefile: `CPUS := 1`
- Successfully built and booted xv6

---

## 2. getprocinfo System Call ✅

**Files Modified**:
- `kernel/syscall.h`: Added `SYS_getprocinfo 22`
- `kernel/syscall.c`: Added handler
- `kernel/sysproc.c`: Implemented function
- `user/user.h`, `user/usys.pl`: User interface

**Test Program**: `user/testproc.c`

**Output**:
```
$ testproc
Testing getprocinfo syscall:
PID: 3, State: 2, Priority: 0
```

**Status**: ✅ Working

---

## 3. MLFQ Scaffolding ✅

**Data Structure** (kernel/proc.h):
```c
struct proc {
  // ... existing fields ...
  int priority;              // 0-3
  int time_slices;           // Time used
  uint64 queue_entry_time;
};
```

**Initialization** (kernel/proc.c - allocproc):
```c
p->priority = 0;
p->time_slices = 0;
p->queue_entry_time = 0;
```

**Scheduler Skeleton** (kernel/proc.c):
- 4-level priority loop
- Round-robin within each level
- Basic structure in place

**Status**: ✅ Complete

---

## 4. Design Document ✅

Created `DESIGN.md` with:
- 4 priority queues (Q0-Q3)
- Time quanta: 1, 2, 4, 8 ticks
- Demotion/promotion policies
- Starvation prevention plan

**Status**: ✅ Complete
