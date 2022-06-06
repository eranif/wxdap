# wxdap
Provide a wxWidgets library for implementing DAP (Debug Adapter Protocol) clients

## Implemented

- Connect
- Initialize
- Launch - This launch request is sent from the client to the debug adapter to start the debuggee
- SetBreakpointsFile - Sets multiple breakpoints for a single source and clears all previous breakpoints in that source
- Threads - The request retrieves a list of all threads.
- Scopes - The request returns the variable scopes for a given stackframe ID


