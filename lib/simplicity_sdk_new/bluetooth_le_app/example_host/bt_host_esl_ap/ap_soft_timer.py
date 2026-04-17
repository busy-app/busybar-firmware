"""
Sorted list-based soft timer implementation
"""

# Copyright 2026 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

import sys
import threading
import time
from auto_importer import AutoImporter
from ap_logger import getLogger

class SoftTimerScheduler:
    """Single-threaded scheduler managing many soft timers efficiently."""

    def __init__(self):
        importer = AutoImporter()
        try:
            SortedList = importer.ensure_import("sortedcontainers", "SortedList")
        except Exception as e:
            self.log.critical("%s Aborting.", e)
            sys.exit(-1)

        self.events = SortedList()
        self.cv = threading.Condition()
        self.running = True
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()
        # periodic wakeup tick – improves timer precision and minimizes drift
        self.ticker = threading.Thread(target=self._ticker, daemon=True)
        self.ticker.start()

    def _ticker(self):
        """Helper thread for stable wakeups and low drift"""
        while self.running:
            time.sleep(0.001)
            with self.cv:
                self.cv.notify_all()

    # Logger
    @property
    def log(self):
        """
        Common AP logger used for all output messages.
        """
        return getLogger("SWT")

    def schedule(self, entry):
        """Insert a timer entry into the sorted list."""
        with self.cv:
            self.events.add(entry)
            self.cv.notify()

    def cancel(self, entry):
        """Remove a timer entry if it exists."""
        with self.cv:
            try:
                self.events.remove(entry)
            except ValueError:
                pass
            self.cv.notify()

    def _run(self):
        """
        Scheduler thread loop.

        Waits for the next scheduled timer and executes it when due.

        Uses a two-level loop to correctly handle condition variable wakeups:
        - The outer loop waits until at least one timer is available.
        - The inner loop repeatedly recomputes the remaining delay, handling
        spurious wakeups (often caused by timer cancellations or restarts
        from other threads) and timing drift on non-RT operating systems.

        This design ensures that:
        - timers fire as close as possible to their scheduled time,
        - cancellations during a wait do not cause race conditions,
        - the scheduler never accesses an empty event list,
        - wakeups caused by notify() or timeout are both handled safely.
        """
        while self.running:
            with self.cv:
                # Wait until at least one event exists
                while self.running and not self.events:
                    self.cv.wait()

                if not self.running:
                    break

                # Main loop: always recompute the remaining delay
                while True:
                    # Check again: events may have been removed while waiting
                    if not self.events:
                        # Go back to outer loop to wait for new events
                        break

                    when, _, timer_obj = self.events[0]
                    now = time.monotonic()
                    delay = when - now

                    if delay <= 0:
                        # Timer expired
                        self.events.pop(0)
                        timer_obj._entry = None
                        break

                    # Wait for the remaining time or until notified
                    self.cv.wait(timeout=delay)

                    # After wakeup, loop again to recompute delay
                    # (This handles spurious wakeups and timing drift)

            # Execute callback outside the lock
            timer_obj._run()

# Global scheduler instance
_global_scheduler = SoftTimerScheduler()

class SoftTimer:
    """API-compatible replacement for threading.Timer using SoftTimerScheduler."""
    def __init__(self, interval, function, args=None, kwargs=None):
        self.interval = interval
        self.function = function
        self.args = args or ()
        self.kwargs = kwargs or {}
        self._entry = None
        self._cancelled = False
        self.daemon = True   # threading.Timer compatibility
        self.name = f"SoftTimer-{id(self)}"

    def join(self, timeout=None):
        """threading.Timer compatibility stub. No thread; no-op"""
        return

    def start(self):
        """Schedule the timer."""
        self._cancelled = False
        when = time.monotonic() + self.interval
        # Use id(self) as a unique tie-breaker
        self._entry = (when, id(self), self)
        _global_scheduler.schedule(self._entry)

    def is_alive(self):
        """API-compatible with threading.Timer.is_alive()."""
        return (not self._cancelled) and (self._entry is not None)

    def cancel(self):
        """Cancel the timer."""
        self._cancelled = True
        if self._entry is not None:
            _global_scheduler.cancel(self._entry)
            self._entry = None

    def restart(self, interval=None):
        self.cancel()
        if interval is not None:
            self.interval = interval
        self.start()

    def _run(self):
        """Internal execution wrapper."""
        if not self._cancelled:
            self.function(*self.args, **self.kwargs)
