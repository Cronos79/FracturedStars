# Sprint 06: Quick Start Guide

## Problem: No Trades Happening

If you see logistics initialized but `Active Routes: 0` and `Trade Requests: 0`, the **economy is not generating shortages yet** because no systems are actively simulating.

## Root Cause

The economy system uses a **two-tier architecture**:
- **Active System**: Real-time economy simulation (production/consumption/shortages)
- **Background Systems**: Time-compressed catch-up only when accessed

**No active system = no economy ticking = no shortages = no trades!**

## Solution: Activate Economy Simulation

### Blueprint Setup (Recommended for Testing)

Add this to your test blueprint's **Event BeginPlay**:

```
1. Get Universe Subsystem
2. Generate Universe (Config: 500 systems, 5 factions, seed 12345)
3. Initialize Economy
4. Initialize Logistics (from universe data)
5. ⚠️ SET ACTIVE ECONOMY SYSTEM ⚠️
   - System ID: 0 (or any valid system 0-499)
   - This starts economy ticking!
6. Create repeating timer (10 seconds):
   - Print Logistics System Status
   - Print Company Statistics
   - Print Cargo Ship Statistics
```

### Critical Function Call

**After InitializeEconomy, you MUST call:**

```cpp
// C++
UniverseSubsystem->SetActiveEconomySystem(0);  // System 0 (first faction core)

// Blueprint
Set Active Economy System
  System ID: 0
```

This tells the economy subsystem to **start actively simulating** that system's markets.

### Why This Matters

**Without active system:**
- Markets are static
- No production/consumption
- Stock levels never change
- No shortages or surpluses
- Logistics has nothing to do

**With active system:**
- ✅ Markets tick every 5-20 seconds
- ✅ Production adds stock
- ✅ Consumption removes stock  
- ✅ Shortage flags get set when stock < 50% target
- ✅ Surplus flags get set when stock > 150% target
- ✅ Logistics detects these and creates trade routes!

## Complete Blueprint Flow

```
Event BeginPlay
├─ Get Game Instance
├─ Get Universe Subsystem
├─ Generate Universe (500 systems, 5 factions, seed 12345)
├─ Initialize Economy
├─ Get Universe Data (for logistics init)
├─ Get Logistics Subsystem  
├─ Initialize Logistics (pass universe data)
├─ ⭐ Set Active Economy System (System ID: 0) ⭐
└─ Set Timer by Event (10s, looping):
   ├─ Print message: "== LOGISTICS STATUS =="
   ├─ Print Logistics System Status
   ├─ Print Company Statistics
   ├─ Print Cargo Ship Statistics
   └─ Print message: "========================"
```

## Expected Behavior After Fix

**First 10 seconds:**
```
Active Routes: 0
Trade Requests: 0
Export Opportunities: 0
```
Economy is just starting to diverge from baseline.

**After 30-60 seconds:**
```
Trade Requests: 5-10
Export Opportunities: 3-8
Active Routes: 0-2
```
Shortages/surpluses forming, routes being matched.

**After 2-3 minutes:**
```
Trade Requests: 15-30
Export Opportunities: 10-20
Active Routes: 5-15
Idle: 0-5 | In Transit: 8-12 | Loading: 1-2
Total Deliveries: 3-10
```
Full trade network active!

## Advanced: Multiple Active Systems

For larger-scale testing, you can simulate multiple systems by:

1. Setting multiple active systems (simulates different player regions)
2. Using `OnPlayerEnterSystem(PlayerController, SystemId)` to dynamically activate
3. Background systems will catch up when accessed

**Performance Note:** Each active system adds ~5-20ms per tick. Limit to 1-3 for testing.

## Debugging: Verify Economy is Ticking

Add this to your timer to confirm economy is working:

```cpp
// Blueprint: Add to your repeating timer
Print Economy Stats

// You should see output like:
// "Markets Active: 10"  (not 0!)
// "Total Shortages: 15"
// "Total Surpluses: 8"
```

If you still see 0 shortages after 60+ seconds:
1. Check that `SetActiveEconomySystem` was called
2. Verify the system ID is valid (0-499)
3. Check logs for economy warnings
4. Increase time scale if needed: `SetTimeScale(240.0)` for 4 minutes per real second

## Quick Fix Checklist

- [ ] Called `GenerateUniverse`
- [ ] Called `InitializeEconomy`  
- [ ] Called `InitializeLogistics`
- [ ] ⚠️ Called `SetActiveEconomySystem(0)` ⚠️ ← **This is what you're missing!**
- [ ] Created repeating timer for status printing
- [ ] Waited 60+ seconds of real time
- [ ] Checked for "Trade Requests" > 0

## Summary

**The fix is one line:**
```cpp
SetActiveEconomySystem(0);  // Add this after InitializeEconomy!
```

Without it, the economy never ticks, markets stay frozen, and logistics has no work to do.

With it, the living trade network will emerge within minutes! 🚀
