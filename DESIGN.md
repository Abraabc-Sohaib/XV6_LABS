# MLFQ Scheduler Design Document

## Team Information
- Sohaib Sohail

## Design Overview

### Queue Structure
- **Number of Queues**: 4 priority levels (Q0, Q1, Q2, Q3)
  - Q0 = Highest priority
  - Q3 = Lowest priority

### Time Quanta
- Q0: 1 timer tick
- Q1: 2 timer ticks
- Q2: 4 timer ticks
- Q3: 8 timer ticks

**Rationale**: Shorter time slices for interactive processes at higher priorities, longer slices for CPU-bound processes at lower priorities.

### Scheduling Rules

#### Demotion Policy
- Process demotes to next lower queue when it **exhausts its time quantum**
- If already at Q3, stays at Q3
- Time slice counter resets after demotion

#### Promotion Policy
- Process promotes to next higher queue when it **performs I/O** (calls sleep)
- If already at Q0, stays at Q0
- Time slice counter resets after promotion
- **Rationale**: I/O-bound processes should be more responsive

#### Starvation Prevention
- **Priority Boosting**: Every 48 ticks, all processes move to Q0
- All time slice counters reset
- **Rationale**: Prevents low-priority processes from starving

### Scheduling Algorithm
```
For each scheduling cycle:
  1. Loop through priority levels Q0 to Q3
  2. Within each level, use round-robin
  3. Run first RUNNABLE process found
  4. On timer interrupt:
     - Increment process time_slices
     - Check if quantum exhausted → demote
     - Check if boost needed → boost all
  5. On sleep (I/O):
     - Promote process to higher priority
```

## Data Structures

### Modified struct proc (kernel/proc.h)
```c
struct proc {
  // ... existing fields ...
  
  int priority;              // Current queue level (0-3)
  int time_slices;          // Ticks used at current priority
  uint64 queue_entry_time;  // When entered current queue (not fully used)
};
```

## Implementation Details

### Files Modified

1. **kernel/proc.h**: Added MLFQ fields to struct proc
2. **kernel/proc.c**: 
   - Modified `allocproc()` to initialize priority fields
   - Modified `scheduler()` for priority-based scheduling
   - Added `boost_all_priorities()` function
   - Modified `sleep()` to promote on I/O
3. **kernel/trap.c**: Modified timer interrupt for quantum tracking and boosting
4. **kernel/sysproc.c**: Implemented `sys_getprocinfo()` and `sys_sleep()`
5. **kernel/syscall.h/c**: Added system call numbers and handlers
6. **user/user.h**: Added user-space declarations
7. **user/usys.pl**: Added system call stubs

### Test Programs Created
- `testproc.c`: Tests getprocinfo syscall
- `cpubound.c`: CPU-intensive workload
- `iobound.c`: I/O-intensive workload
- `mlfqtest.c`: Comprehensive mixed workload test

## Expected Behavior

### CPU-bound Process
- Starts at priority 0
- Quickly demotes to priority 3 (within seconds)
- Stays at priority 3 until next boost
- Less responsive, lower throughput

### I/O-bound Process
- Starts at priority 0
- Performs I/O operations (sleep)
- Gets promoted back to high priority after each I/O
- Stays at priority 0 or 1
- More responsive, higher apparent throughput

### Mixed Workload
- I/O-bound processes get more CPU time proportionally
- CPU-bound processes run but at lower priority
- No starvation due to periodic boosting

## Testing Results

[Fill in after testing]

### Test 1: getprocinfo
- Shows processes with different priority levels
- Time slices increment correctly

### Test 2: cpubound behavior
- [Describe what you observed]

### Test 3: iobound behavior
- [Describe what you observed]

### Test 4: Mixed workload (mlfqtest)
- [Describe the interaction between CPU and I/O processes]

## Challenges Faced

1. Understanding xv6 scheduler and context switching
2. Correctly implementing time quantum tracking
3. Ensuring locks are used properly
4. [Add any other challenges you faced]

## What We Learned

- Deep understanding of MLFQ scheduling algorithm
- Kernel-level C programming experience
- Operating system internals and process management
- Debugging techniques in OS development
