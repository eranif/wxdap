# wxdap
Provide a wxWidgets library for implementing DAP (Debug Adapter Protocol) clients

## Implemented

### Requests

#### Mandatory requests

- [x] Connect
- [x] Initialize
- [x] Launch - This launch request is sent from the client to the debug adapter to start the debuggee
- [x] SetBreakpointsFile - Sets multiple breakpoints for a single source and clears all previous breakpoints in that source
- [x] Threads - The request retrieves a list of all threads.
- [x] Scopes - The request returns the variable scopes for a given stackframe ID (this does not return the variables themselves, but only their groups, like "Locals", "Registers" etc)
- [x] GetFrames - return list of frames for a given thread ID
- [x] Continue - continue the execution
- [x] Next - executes one step for the specified thread
- [X] StepIn - resumes the given thread to step into a function/method and allows all other threads to run freely by resuming them
- [X] StepOut - The request resumes the given thread to step out (return) from a function/method and allows all other threads to run freely by resuming them
- [ ] Pause - pause the debugger execution
- [ ] BreakpointLocations - returns all possible locations for source breakpoints in a given range
- [X] SetFunctionBreakpoints - Replaces all existing function breakpoints with new function breakpoints
- [X] Variables - return list of variables

#### Lower priority requests

- [ ] NextInstruction - executes one instruction for the specified thread
- [ ] Goto - sets the location where the debuggee will continue to run. his makes it possible to skip the execution of code or to execute code again
- [ ] ReadMemory - Reads bytes from memory at the provided location
- [ ] Disassemble - Disassembles code stored at the provided location

### Events

- [x] Stopped - the execution stopped due to ... (breakpoint hit, exception, step etc)
- [x] Terminated - the debugging session terminated
- [x] Exited - the debuggee process exited
- [x] Initialized - dap server is initialized
- [x] Process - the debuggee process started
- [x] Output - The event indicates that the target has produced some output
