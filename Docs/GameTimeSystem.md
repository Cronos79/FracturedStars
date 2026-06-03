# Game Time System

## Overview

The game time system provides a persistent, scalable calendar and clock for the universe simulation. It tracks in-game date and time, advancing automatically based on configurable time scale, and integrates with the economy system for catch-up simulation.

## Core Components

### FUniverseTime Struct

Located in `UniverseTypes.h`, this struct stores the current game date and time:

```cpp
struct FUniverseTime
{
	int32 Year;              // e.g., 2094
	int32 Month;             // 1-12 (January = 1)
	int32 Day;               // 1-31
	int32 Hour;              // 0-23
	int32 Minute;            // 0-59
	int32 Second;            // 0-59
	double TotalElapsedSeconds; // Total seconds since universe creation
}
```

**Note:** Named `FUniverseTime` instead of `FGameTime` to avoid conflict with Unreal Engine's built-in `FGameTime` frame timing struct.

### Time Configuration

Set in `FUniverseConfig`:

- **StartYear**: Initial year (default: 2094)
- **StartMonth**: Initial month (default: 7 = July)
- **StartDay**: Initial day (default: 8)
- **TimeScale**: Real time to game time multiplier (default: 24.0)
  - `24.0` means 1 real second = 24 game seconds
  - Therefore: 1 real hour = 1 game day (24 hours)
  - 1 real day = 24 game days

## Time Advancement

Time automatically advances via a timer in `UUniverseSubsystem`:

- **Update Rate**: 10Hz (every 0.1 real seconds)
- **Advancement Formula**: `DeltaGameSeconds = RealDeltaTime × TimeScale`
- **Calendar Logic**: Automatically handles rollover for seconds, minutes, hours, days, months, and years
- **Leap Years**: Supported using standard Gregorian calendar rules

### Time Control Functions

```cpp
// Pause/resume time
void SetTimePaused(bool bPaused);
bool IsTimePaused() const;

// Adjust time speed
void SetTimeScale(float NewScale);
float GetTimeScale() const;
```

## Querying Time

### C++ API

```cpp
UUniverseSubsystem* Universe = GetGameInstance()->GetSubsystem<UUniverseSubsystem>();

// Get complete time structure
FUniverseTime CurrentTime = Universe->GetGameTime();

// Get formatted strings
FString Date = Universe->GetFormattedDate();      // "July 8, 2094"
FString Time = Universe->GetFormattedTime();      // "14:32:05"
FString DateTime = Universe->GetFormattedDateTime(); // "July 8, 2094 14:32:05"

// Get elapsed days (useful for economy/events)
double Days = Universe->GetElapsedDays();
```

### Blueprint API

All time query functions are exposed to Blueprints:

- **GetGameTime**: Returns `FUniverseTime` struct (BlueprintPure)
- **GetFormattedDate**: Returns date string (BlueprintPure)
- **GetFormattedTime**: Returns time string (BlueprintPure)
- **GetFormattedDateTime**: Returns combined string (BlueprintPure)
- **GetElapsedDays**: Returns total elapsed days (BlueprintPure)
- **SetTimeScale**: Change time speed (BlueprintCallable)
- **SetTimePaused**: Pause/resume time (BlueprintCallable)

### UMG Widget Example

```cpp
// In your UMG widget's Tick or via binding:
UUniverseSubsystem* Universe = GetWorld()->GetGameInstance()->GetSubsystem<UUniverseSubsystem>();
if (Universe)
{
	DateText->SetText(FText::FromString(Universe->GetFormattedDate()));
	TimeText->SetText(FText::FromString(Universe->GetFormattedTime()));
}
```

## Economy Integration

The economy system uses `TotalElapsedSeconds` for catch-up simulation:

```cpp
// Market last update times are stored as doubles (seconds)
double TimeDelta = UniverseData.CurrentTime.TotalElapsedSeconds - Location.Market.LastUpdateTime;

// Simulate economy for that many hours
float SimulationHours = TimeDelta / 3600.0f;
```

This allows the economy to accurately catch up when:
- Player enters a new system
- Game is loaded from save
- Background systems need to update

## Testing and Debugging

### Console Commands

```cpp
// Print detailed time information
PrintTimeInfo()
```

This outputs:
- Current date and time (formatted)
- Elapsed days
- Total elapsed seconds
- Current time scale
- Pause state

### Time Scale Examples

| Time Scale | Real → Game Time | Use Case |
|-----------|-----------------|----------|
| 1.0 | 1:1 (real-time) | Testing, slow gameplay |
| 24.0 | 1 hour = 1 day | **Default**, balanced progression |
| 60.0 | 1 hour = 2.5 days | Fast progression |
| 360.0 | 1 hour = 15 days | Very fast, long-term simulation |

## Save/Load Integration

**Status:** Not yet implemented (Sprint 4+)

When save/load is added, `FUniverseTime` will be serialized with `FUniverseData`:

```cpp
// Future save format:
{
	"CurrentTime": {
		"Year": 2094,
		"Month": 7,
		"Day": 15,
		"Hour": 8,
		"Minute": 23,
		"Second": 45,
		"TotalElapsedSeconds": 612225.0
	}
}
```

## Performance Notes

- **Timer Overhead**: Minimal - simple math operations every 0.1s
- **No Frame Dependency**: Uses timer, not Tick, so consistent regardless of FPS
- **Thread Safe**: All time updates happen on game thread
- **Pause Aware**: Respects both custom pause flag and world pause state

## Calendar Details

### Month Names

English month names used:
- January, February, March, April, May, June
- July, August, September, October, November, December

### Leap Year Rules

Standard Gregorian calendar:
- Divisible by 4: Leap year
- Divisible by 100: Not a leap year
- Divisible by 400: Leap year

Example: 2096 is a leap year (2094 is not)

### Days Per Month

- Jan, Mar, May, Jul, Aug, Oct, Dec: 31 days
- Apr, Jun, Sep, Nov: 30 days
- Feb: 28 days (29 in leap years)

## Future Enhancements

Potential additions for future sprints:

1. **Time Events System**: Trigger events at specific dates/times
2. **Season Support**: Calculate seasons based on day of year
3. **Custom Calendars**: Support for non-Earth calendar systems
4. **Time Zones**: Per-system or per-planet time zones
5. **Historical Log**: Track major events by date
6. **Time Compression**: Dynamic time scale based on player activity

## Related Systems

- **Economy Subsystem**: Uses time for catch-up simulation and market updates
- **Universe Subsystem**: Owns and manages time state
- **Future Save System**: Will persist time state
- **Future Event System**: Will use time for scheduling

## Technical Notes

### Why Not Use Engine GameTime?

Unreal Engine's `FGameTime` struct is for frame timing (world time vs real time for pause/dilation). Our `FUniverseTime` is for in-universe calendar tracking - completely different purposes.

### Thread Safety

All time functions are called on the game thread. The timer delegate captures `this` safely as the subsystem exists for the entire game instance lifetime.

### Precision

- Uses `double` for `TotalElapsedSeconds` to support years of game time without precision loss
- Uses `int32` for date/time components (sufficient for thousands of years)
- 10Hz update rate provides smooth time progression without excessive overhead

---

**Implementation Date:** Sprint 3 (Game Time System)  
**Last Updated:** Initial implementation  
**See Also:** `Sprint03_EconomyCore_Spec.md`, `UniverseTypes.h`
