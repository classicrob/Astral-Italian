# Headless breakpoint ordering

DrawSyncManager snapshots FIFO write position P in pushBreakPoint and posts it
to its own OS thread. Before that thread dequeues the pointer, the game may write
a 5-byte token BP and process its event at P+5. The old null GXEnableBreakPt then
reported a hit at P immediately. The platform's monotonic event check correctly
aborted: P was reported after P+5. An interrupt lock around each individual report
does not turn an old requested address into a newly reached processor cursor.

The null adapter now has separate written and processed cursors and an ordered
queue of sync commands. Its synchronous processor runs under the OS interrupt
lock: command write and event registration complete before processing is allowed.
A future breakpoint holds processing and retains later events while writes can
continue. Clearing the breakpoint resumes that queue. A breakpoint behind the
cursor never fires retroactively. Abort discards only unprocessed commands and
uses the existing platform lost-draw-done accounting. The platform event-order
fatal check remains unchanged.

This follows the source contract in Dolphin revision
409881c34357d0f105fd473167d15ab0bd9c1628,
[CommandProcessor.cpp lines 442–474](https://github.com/dolphin-emu/dolphin/blob/409881c34357d0f105fd473167d15ab0bd9c1628/Source/Core/VideoCommon/CommandProcessor.cpp#L442):
the enabled breakpoint flag is set when CPBreakpoint equals CPReadPointer and
cleared when unequal. The repository SDK GXEnableBreakPt (GXFifo.c:377–391)
programs the address and enables the breakpoint; it does not call the handler.
The native FIFO uses monotonically increasing opaque positions rather than a
physical ring, so an already passed address cannot be reached again by wrapping.

The deterministic regression forces the exact delayed-arm interleaving across
two threads. Its old-path control reports P after P+5 and is rejected; the fixed
processor keeps the cursor at P+5 without a hit. Other cases cover token-before-
breakpoint ordering, held events, resume, abort and 10,000 repeated stale arms.
The existing platform tests still require genuinely reversed reports to abort.
This fixes the headless adapter, not an unproven claim about Aurora's processor.
