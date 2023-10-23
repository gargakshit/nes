#ifndef NES_CONFIG_H
#define NES_CONFIG_H

#ifdef NES_CPU_RT
// Warn.
#warning Building with CPU RT tracing. Will impact performance.

// Enable runtime CPU sanity checks.
// If we reach an impossible state, we panic. It is recommended that we don't
// build with sanity checks sweep_enabled as those cost a lot of performance.
#define NES_CPU_RT_SANITY

// Enable CPU tracing.
// Will print tick ticks, executed instructions, IRQs, NMIs, RSTs and register
// dumps on context switches.
#define NES_CPU_RT_TRACE
#endif // NES_CPU_RT

#endif // NES_CONFIG_H
