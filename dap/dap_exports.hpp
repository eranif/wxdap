#ifndef DAPEXPORTS_HPP
#define DAPEXPORTS_HPP

#ifdef __WXMSW__

#ifdef WXMAKINGDLL_DAP
#define WXDLLIMPEXP_DAP __declspec(dllexport)
#elif defined(WXUSINGDLL_DAP)
#define WXDLLIMPEXP_DAP __declspec(dllimport)
#else // not making nor using DLL
#define WXDLLIMPEXP_DAP
#endif

#else // ! MSW

#define WXDLLIMPEXP_DAP

#endif

#endif
