//
// Created by raphael on 10/04/18.
//

#include "common.h"
#include "vm.h"
#include <stdio.h>
#include <assert.h>
#include "value.h"

#ifdef DEBUG_TRACE_EXECUTION
#include "trace.h"
#endif
#ifdef BENCHMARK_TIMINGS
#include "benchmark.h"
#endif

#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.object.values[READ_BYTE()])
#define GOTO(a) vm.ip = vm.chunk->code + (a)

VM vm;

static void resetStack() {
    vm.sp = vm.stack;
    vm.fp = vm.sp;
}

void initVM(unsigned int flags, int (*printf)(const char *, ...), ValueArray *args) {
    resetStack();
    vm.maxstack = vm.stack + STACK_MAX;
    vm.flags = flags;
    vm.numObjects = 0;
    vm.maxObjects = INITIAL_GC_THRESHOLD;
    vm.firstObject = NULL;
    vm.printf = printf;
    vm.args = args;
    vec_init(&vm.fcs);
}

bool hasFlag(VMFlags flag) {
    return (vm.flags & flag) == flag;
}

VM *getVM() {
    return &vm;
}

void freeVM() {
    resetStack();
    gc();
}

void registerObject(Object *object) {
    if (vm.numObjects >= vm.maxObjects) gc();

    object->marked = 0;
    object->next = vm.firstObject;
    vm.firstObject = object;

    vm.numObjects++;
}

void markAll() {
    for (Value *v = vm.stack; v < vm.sp; v++) {
        markValue(v);
    }
}

void markValue(Value *v) {
    Value vl = *accessRefp(v);
    if (typecheck(vl, T_OBJECT) || typecheck(vl, T_ARRAY)) {
        Object *object = v2o(vl);

        mark(object);
    }
}

void mark(Object *object) {
    if (object->marked) return;

    object->marked = 1;

    for (int i = 0; i < object->size; ++i) {
        markValue(&object->values[i]);
    }
}

void sweep() {
    Object **object = &vm.firstObject;
    while (*object) {
        if (!(*object)->marked) {
            /* This object wasn't reached, so remove it from the list
               and free it. */
            Object *unreached = *object;

            *object = unreached->next;
            free(unreached);
            free(unreached->values);
            vm.numObjects--;
        } else {
            /* This object was reached, so unmark it (for the next GC)
               and move on to the next. */
            (*object)->marked = 0;
            object = &(*object)->next;
        }
    }
}

void gc() {
    markAll();
    sweep();

    vm.maxObjects = vm.numObjects < INITIAL_GC_THRESHOLD ? INITIAL_GC_THRESHOLD : vm.numObjects * 2;
}

#define VM_BINARY(code, f, rf, op) \
case TARGET(code): { \
    Value l = pop(); \
    Value r = pop(); \
    push(rf##2v(v2##f(l) op v2##f(r))); \
    NEXT(); \
}

#define VM_BINARY_EQ(code, f, op) \
case TARGET(code): { \
    OP_T eq = READ_BYTE(); \
    Value l = pop(); \
    Value r = pop(); \
    if (eq) { \
        push(b2v(v2##f(l) op##= v2##f(r))); \
    } else { \
        push(b2v(v2##f(l) op v2##f(r))); \
    }\
    NEXT(); \
}

#define VM_MATH_OPS(t, f) \
    VM_BINARY(OP_##t##ADD, f, f, +) \
    VM_BINARY(OP_##t##SUB, f, f, -) \
    VM_BINARY(OP_##t##MUL, f, f, *) \
    VM_BINARY(OP_##t##DIV, f, f, /) \
    VM_BINARY_EQ(OP_##t##LT, f, <) \
    VM_BINARY_EQ(OP_##t##GT, f, >)

#ifdef USE_COMPUTED_GOTO
    #define OP_LABEL(op) label_vm_##op
    #define OP_LABEL_ADDR(op) &&OP_LABEL(op),
    #define TARGET(op) op: OP_LABEL(op)
    #define NEXT_GOTO() goto *addrs[READ_BYTE()]

    #ifdef RISOTTO_DEBUG_LIB
        #define NEXT_INTERNAL() NEXT_GOTO()
        #define NEXT() goto next_opcode
    #else
        #define NEXT() NEXT_GOTO()
    #endif
#else
    #define TARGET(op) op

    #ifdef RISOTTO_DEBUG_LIB
        #define NEXT_INTERNAL() goto main_loop
        #define NEXT() goto next_opcode
    #else
        #define NEXT() break
    #endif
#endif

static InterpretResult run() {
#ifdef USE_COMPUTED_GOTO
    static const void *addrs[] = {OPCODES(OP_LABEL_ADDR)};
#endif

#ifdef BENCHMARK_TIMINGS
    BenchmarkData benchmarkData = {};
#endif

    main_loop:
    for (;;) {
        OP_T instruction = READ_BYTE();

        switch (instruction) {
            case TARGET(OP_CONST):
            {
                Value constant = READ_CONSTANT();
                push(constant);
                NEXT();
            }
            case TARGET(OP_JUMP):
            {
                OP_T addr = READ_BYTE();

                GOTO(addr);

                NEXT();
            }
            case TARGET(OP_JUMPT):
            {
                OP_T addr = READ_BYTE();
                bool b = v2b(pop());

                if (b == true) {
                    GOTO(addr);
                }
                NEXT();
            }
            case TARGET(OP_JUMPF):
            {
                OP_T addr = READ_BYTE();
                bool b = v2b(pop());

                if (b == false) {
                    GOTO(addr);
                }
                NEXT();
            }
            case TARGET(OP_RESOLVE_ADDR):
            {
                OP_T vaddr = READ_BYTE();

                Value v = accessRef(pop());

                if (v.tc->vtable == NULL) {
                    ERROR("vtable is null")
                }

                bool found = false;
                int i;
                vtable_entry *entry;
                vec_foreach_ptr(&v.tc->vtable->addrs, entry, i) {
                        if (entry->vaddr == vaddr) {
                            push(entry->addr);
                            found = true;
                            break;
                        }
                    }

                if (!found) {
                    ERROR("Unable to find addr")
                }
                NEXT();
            }
            case TARGET(OP_CALL):
            {
                { // Forces VLA in different scope than goto
                    int argc = READ_BYTE(); // args count
                    int retc = READ_BYTE(); // return values count

                    bool refs[argc];
                    for (int i = 0; i < argc; ++i) {
                        refs[i] = (bool) READ_BYTE();
                    }

                    Value f = accessRef(pop());

                    switch (TGET(f)) {
                        case T_INT: { // Function
                            int addr = v2i(f);

                            FunctionCall fc = (FunctionCall) {
                                    .argc = argc,
                                    .retc = retc,
                                    .ip = vm.ip,
                                    .fp = vm.fp,
                                    .sp = vm.sp,
                            };
                            vec_push(&vm.fcs, fc);
                            cframe();

                            GOTO(addr);

                            for (int i = 0; i < argc; ++i) {
                                Value *a = vm.fp - 2 - i;

                                if (refs[i] == true) {
                                    push(vp2v(a));
                                } else {
                                    push(copy(*a));
                                }
                            }

                            break;
                        }
                        case T_P: { // Native function
                            Value args[argc];
                            for (int i = 0; i < argc; ++i) {
                                args[i] = pop();
                            }

                            NativeFunction fun = v2p(f);

                            Value returnValues[retc];
                            fun(args, argc, returnValues);

                            pushm(returnValues, retc);

                            break;
                        }
                        default:
                        ERROR("Unhandled function type")
                    }
                }

                NEXT();
            }
            case TARGET(OP_FRAME):
            {
                cframe();
                NEXT();
            }
            case TARGET(OP_FRAME_END):
            {
                dframe();
                NEXT();
            }
            case TARGET(OP_RETURN):
            {
                { // Forces VLA in different scope than goto
                    FunctionCall fc = vec_pop(&vm.fcs);
                    int rc = fc.retc;

                    Value rvals[rc];
                    for (int i = rc - 1; i >= 0; --i) {
                        rvals[i] = copy(pop());
                    }

                    vm.ip = fc.ip;
                    vm.sp = fc.sp;
                    vm.fp = fc.fp;

                    vm.sp -= fc.argc;

                    pushm(rvals, rc);
                }
                NEXT();
            }
            case TARGET(OP_LOAD):
            {
                OP_T addr = READ_BYTE();

                Value *vp = vm.fp + addr;

                push(vp2v(vp));
                NEXT();
            }
            case TARGET(OP_LOAD_STACK):
            {
                OP_T addr = READ_BYTE();

                Value *v = vm.sp - 1 - addr;

                push(vp2v(v));
                NEXT();
            }
            case TARGET(OP_LOAD_LOCAL):
            {
                OP_T dist = READ_BYTE();
                OP_T addr = READ_BYTE();

                Value *fp = vm.fp;
                for (int i = 0; i < dist; ++i) {
                    Value *a = fp - 1;
                    fp = v2p(*a);
                }

                Value *vp = fp + addr;

                assert(vp < vm.sp);
                assert(vp >= vm.stack);

                push(vp2v(vp));
                NEXT();
            }
            case TARGET(OP_LOAD_INSTANCE):
            {
                OP_T addr = READ_BYTE();
                loadInstance(addr);
                NEXT();
            }
            case TARGET(OP_NEW):
            {
                ValueTypeContainer *tc = v2p(READ_CONSTANT());
                OP_T size = READ_BYTE();

                Object *instance = malloc(sizeof(*instance));
                instance->size = size;
                instance->values = malloc(size * sizeof(*instance->values));

                for (int i = 0; i < size; ++i) {
                    instance->values[i] = n2v();
                }

                registerObject(instance);

                Value ov = o2v(instance, tc);

                push(ov);
                NEXT();
            }
            case TARGET(OP_SET):
            {
                Value o = pop();
                Value *t = vm.sp - 1;

                set(o, t);

                NEXT();
            }
            case TARGET(OP_POP):
            {
                int n = READ_BYTE();

                for (int i = 0; i < n; ++i) {
                    popp();
                }

                NEXT();
            }
            case TARGET(OP_COPY):
            {
                push(copy(pop()));
                NEXT();
            }
            case TARGET(OP_NIL):
            {
                push(n2v());
                NEXT();
            }
            case TARGET(OP_ARRAY):
            {
                ValueTypeContainer *tc = v2p(READ_CONSTANT());
                int elemsc = READ_BYTE();

                ValueArray *array = malloc(sizeof(*array));
                initValueArray(array);
                registerObject((Object *) array);

                for (int i = 0; i < elemsc; ++i) {
                    writeValueArray(array, copy(pop()));
                }

                push(a2v(array, tc));
                NEXT();
            }
            case TARGET(OP_TRUE):
            {
                push(b2v(true));
                NEXT();
            }
            case TARGET(OP_FALSE):
            {
                push(b2v(false));
                NEXT();
            }
            case TARGET(OP_EQ_NIL):
            {
                Value v = pop();

                push(b2v(typecheck(v, T_NIL)));
                NEXT();
            }
            case TARGET(OP_NEQ_NIL):
            {
                Value v = pop();

                push(b2v(!typecheck(v, T_NIL)));
                NEXT();
            }
            case TARGET(OP_EQ):
            {
                Value l = pop();
                Value r = pop();

                push(b2v(veq(l, r)));
                NEXT();
            }
            case TARGET(OP_NEQ):
            {
                Value l = pop();
                Value r = pop();

                push(b2v(!veq(l, r)));
                NEXT();
            }
            VM_MATH_OPS(I, i)
            VM_BINARY(OP_IMOD, i, i, %)
            VM_MATH_OPS(D, d)
            VM_BINARY(OP_B_AND, i, i, &)
            VM_BINARY(OP_B_OR, i, i, |)
            VM_BINARY(OP_B_XOR, i, i, ^)
            VM_BINARY(OP_B_SHIFTL, i, i, <<)
            VM_BINARY(OP_B_SHIFTR, i, i, >>)
            case TARGET(OP_B_NOT):
            {
                Value l = pop();
                push(i2v(~v2i(l)));
                NEXT();
            }
            case TARGET(OP_I2D):
            {
                Value l = pop();
                push(d2v(v2i(l)));
                NEXT();
            }
            case TARGET(OP_D2I):
            {
                Value l = pop();
                push(i2v(v2d(l)));
                NEXT();
            }
            case TARGET(OP_END):
            {
#ifdef BENCHMARK_TIMINGS
                benchmarkPrint(&benchmarkData);
#endif

                return INTERPRET_OK;
            }
            default:
                printf("Unknown op code: %d \n", instruction);
                return INTERPRET_RUNTIME_ERROR;
        }
    }

    // Debug next opcode, allows for interception
#ifdef RISOTTO_DEBUG_LIB
    next_opcode:
    {
#ifdef DEBUG_TRACE_EXECUTION
        printTrace();
#endif
#ifdef BENCHMARK_TIMINGS
        benchmarkRuntime(&benchmarkData);
#endif
        NEXT_INTERNAL();
    }
#endif
}

InterpretResult interpret(Chunk *chunk, long addr) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code + addr;

    InterpretResult result = run();
    return result;
}

void loadInstance(int index) {
    Object *array = v2o(pop());

    unsigned int offset;
    if (index < 0) {
        offset = index + array->size;
    } else {
        offset = index;
    }

    Value *p = array->values + offset;

    push(vp2v(p));
}

void push(Value value) {
    if (vm.sp > vm.maxstack) {
        ERROR("Stack overflow")
    }

    *vm.sp++ = value;
}

void pushm(Value *values, unsigned int length) {
    if (vm.sp + length > vm.maxstack) {
        ERROR("Stack overflow")
    }

    memcpy(vm.sp, values, sizeof(Value) * length);
    vm.sp += length;
}

void cframe() {
    push(p2v(vm.fp));
    vm.fp = vm.sp;
}

void dframe() {
    vm.sp = vm.fp;
    vm.fp = v2p(pop());
}

Value pop() {
    return *popp();
}

Value *popp() {
    if (vm.sp == vm.stack) {
        ERROR("Stack underflow")
    }

    return --vm.sp;
}
