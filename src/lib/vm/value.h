//
// Created by raphael on 10/04/18.
//

#ifndef RISOTTOVM_VALUE_H
#define RISOTTOVM_VALUE_H

#include "common.h"
#include "../lib/vec/src/vec.h"

#define DGET(value, type) (value).data._##type
#define TGET(value) (value).type
#define NEW_VALUE(t, tu, value) (Value) {.type = t, .data={._##tu = value}}
#define ACCESS_REF(value) value = accessRef(value)

typedef enum {
    T_NIL,
    T_UINT,
    T_INT,
    T_DOUBLE,
    T_P,
    T_STR,
    T_OBJECT,
    T_BOOL,
    T_ARRAY,
    T_VALUE_P,
} ValueType;

typedef union {
    unsigned int _uint;
    int _int;
    double _double;
    void *_p;
    const char *_str;
    bool _bool;
} ValueData;

typedef struct vtable vtable;

typedef struct {
    ValueData data;
    ValueType type;
    vtable *vtable;
} Value;

// This is super confusing syntax:
// Creates a type `NativeFunction` which is a function `NativeFunctionReturn (Value *, int)`
typedef void (*NativeFunction)(Value args[], int argsc, Value ret[]);

typedef struct {
    int vaddr;
    Value addr;
} vtable_entry;

typedef vec_t(vtable_entry) vtable_entry_vec_t;

typedef struct {
    int argc;
    int retc;

    OP_T *ip;
    Value *sp;
    Value *fp;
} FunctionCall;

typedef vec_t(FunctionCall) function_call_vec_t;

struct vtable {
    unsigned int size;
    vtable_entry_vec_t addrs;
};

typedef struct Object Object;

struct Object {
    Object *next;
    unsigned char marked;
    Value *values;
    unsigned int size;
};

typedef struct {
    Object object;
    unsigned int capacity;
} ValueArray;

void initValueArray(ValueArray *array);

void writeValueArray(ValueArray *array, Value value);

void freeValueArray(ValueArray *array);

Value n2v();

Value p2v(void *v);

Value ui2v(unsigned int v);

Value i2v(int v);

Value d2v(double v);

Value s2v(const char *v);

Value o2v(Object *v);

Value b2v(bool b);

Value vp2v(Value *);

Value a2v(ValueArray *);

void *v2p(Value v);

unsigned int v2ui(Value v);

int v2i(Value v);

double v2d(Value v);

const char *v2s(Value v);

Object *v2o(Value v);

bool v2b(Value v);

ValueArray *v2a(Value v);

Value copy(Value v);

bool typecheck(Value value, ValueType type);

Value accessRef(Value v);

Value *accessRefp(Value *v);

#endif //RISOTTOVM_VALUE_H
