# MLFQ Scheduler - Final Report

**Student**: Sohaib Sohail 28994
**Date**: 29-November-2025
**Project**: Multi-Level Feedback Queue on xv6-RISC-V

---

## Summary

Successfully implemented MLFQ scheduler in xv6 with:
- 4 priority queues (Q0-Q3)
- Dynamic time quanta (1, 2, 4, 8 ticks)
- Priority adjustment based on behavior
- Starvation prevention via boosting

---

## Implementation

### Core Features ✅
- Priority-based scheduling
- Time quantum enforcement
- Demotion on quantum exhaustion
- Promotion on I/O operations
- Priority boost every 48 ticks
- `getprocinfo()` and `sleep()` syscalls

### Modified Files
1. **kernel/proc.h**: Added priority fields
2. **kernel/proc.c**: Modified scheduler, allocproc, sleep, added boost
3. **kernel/trap.c**: Timer interrupt handling
4. **kernel/sysproc.c**: System calls
5. **syscall.h/c, user.h, usys.pl**: Syscall infrastructure

### Test Programs
- `testproc`: Test getprocinfo
- `cpubound`: CPU-intensive workload
- `iobound`: I/O-intensive workload
- `mlfqtest`: Mixed workload test

**Code Added**: ~150 lines

---

## Testing Results

### CPU-bound Process
- ✅ Demotes to Q3 quickly
- ✅ Lower responsiveness (expected)
- ✅ Completes successfully

### I/O-bound Process
- ✅ Stays at Q0/Q1
- ✅ High responsiveness
- ✅ Fast completion

### Mixed Workload
- ✅ I/O processes prioritized
- ✅ CPU processes make progress
- ✅ No starvation
- ✅ Fair scheduling

---

## Algorithm Summary

**Scheduler**:
```
Loop priority 0→3:
  Find RUNNABLE at this priority
  Run it
  Restart from Q0
```

**Timer**:
```
time_slices++
If quantum exceeded: demote, reset
Every 48 ticks: boost all to Q0
```

**I/O**:
```
On sleep: promote, reset time_slices
```

---

## Challenges & Solutions

1. **Context switching**: Read xv6 book, traced code
2. **Timer integration**: Found `which_dev == 2` condition
3. **Lock management**: Followed xv6 patterns
4. **System calls**: Added complete infrastructure


## Results

| Metric       | Result |
|--------------|--------|
| All features | ✅ Working |
| CPU demotion | ✅ Yes |
| I/O promotion| ✅ Yes |
| Starvation   | ✅ None |
| Tests passed | ✅ All |


