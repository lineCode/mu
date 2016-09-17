/*
 * First class functions
 */

#ifndef MU_FN_H
#define MU_FN_H
#include "mu.h"


// Definition of C Function types
struct code;
typedef mc_t mbfn_t(mu_t frame[MU_FRAME]);
typedef mc_t msbfn_t(mu_t closure, mu_t frame[MU_FRAME]);


// Creation functions
mu_t fn_create(struct code *c, mu_t closure);

// Conversion operations
mu_t fn_frombfn(mc_t args, mbfn_t *bfn);
mu_t fn_fromsbfn(mc_t args, msbfn_t *sbfn, mu_t closure);

// Function calls
mc_t fn_tcall(mu_t f, mc_t fc, mu_t *frame);
void fn_fcall(mu_t f, mc_t fc, mu_t *frame);
mu_t fn_vcall(mu_t f, mc_t fc, va_list args);
mu_t fn_call(mu_t f, mc_t fc, ...);

// Iteration
bool fn_next(mu_t f, mc_t fc, mu_t *frame);


// Function tags
enum fn_flags {
    FN_BUILTIN = 1 << 0, // C builtin function
    FN_SCOPED  = 1 << 1, // Closure attached to function
    FN_WEAK    = 1 << 2, // Closure is weakly referenced
};

// Definition of code structure used to represent the
// executable component of Mu functions.
struct code {
    mref_t ref;     // reference count
    mbyte_t args;   // argument count
    mbyte_t flags;  // function flags
    muintq_t regs;  // number of registers
    muintq_t scope; // size of scope

    mlen_t icount;  // number of immediate values
    mlen_t fcount;  // number of code objects
    mlen_t bcount;  // number of bytecode instructions

    // data that follows code header
    // immediate values
    // nested code objects
    // bytecode
};

// Code reference counting
mu_inline struct code *code_inc(struct code *c) {
    ref_inc(c);
    return c;
}

mu_inline void code_dec(struct code *c) {
    extern void code_destroy(struct code *);
    if (ref_dec(c)) {
        code_destroy(c);
    }
}

// Code access functions
mu_inline mu_t *code_imms(struct code *c) {
    return (mu_t *)(c + 1);
}

mu_inline struct code **code_fns(struct code *c) {
    return (struct code **)((mu_t *)(c + 1) + c->icount);
}

mu_inline void *code_bcode(struct code *c) {
    return (void *)((struct code **)((mu_t *)(c + 1) + c->icount) + c->fcount);
}


// Definition of the function type
//
// Functions are stored as function pointers paired with closures.
// Additionally several flags are defined to specify how the
// function should be called.
struct fn {
    mref_t ref;     // reference count
    mbyte_t args;   // argument count
    mbyte_t flags;  // function flags

    mu_t closure;  // function closure

    union {
        mbfn_t *bfn;       // c function
        msbfn_t *sbfn;     // scoped c function
        struct code *code; // compiled mu code
    } fn;
};

// Function creating functions
mu_inline mu_t mbfn(mc_t args, mbfn_t *bfn) {
    return fn_frombfn(args, bfn);
}

mu_inline mu_t msbfn(mc_t args, msbfn_t *sbfn, mu_t closure) {
    return fn_fromsbfn(args, sbfn, closure);
}

// Function reference counting
mu_inline mu_t fn_inc(mu_t f) {
    mu_assert(mu_isfn(f));
    ref_inc(f);
    return f;
}

mu_inline void fn_dec(mu_t f) {
    mu_assert(mu_isfn(f));
    extern void fn_destroy(mu_t);
    if (ref_dec(f)) {
        fn_destroy(f);
    }
}

// Function access
mu_inline bool fn_isbuiltin(mu_t m) {
    return ((struct fn *)((muint_t)m - MTFN))->flags & FN_BUILTIN;
}

mu_inline struct code *fn_code(mu_t m) {
    return code_inc(((struct fn *)((muint_t)m - MTFN))->fn.code);
}

mu_inline mu_t fn_closure(mu_t m) {
    return mu_inc(((struct fn *)((muint_t)m - MTFN))->closure);
}


// Function constant macro
#define MBFN(name, args, bfn)                                               \
mu_pure mu_t name(void) {                                                   \
    static const struct fn inst = {0, args, FN_BUILTIN, 0, {bfn}};          \
    return (mu_t)((muint_t)&inst + MTFN);                                   \
}


#endif
