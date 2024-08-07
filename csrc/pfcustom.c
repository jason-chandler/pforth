/* @(#) pfcustom.c 98/01/26 1.3 */

#ifndef PF_USER_CUSTOM

/***************************************************************
** Call Custom Functions for pForth
**
** Create a file similar to this and compile it into pForth
** by setting -DPF_USER_CUSTOM="mycustom.c"
**
** Using this, you could, for example, call X11 from Forth.
** See "pf_cglue.c" for more information.
**
** Author: Phil Burk
** Copyright 1994 3DO, Phil Burk, Larry Polansky, David Rosenboom
**
** Permission to use, copy, modify, and/or distribute this
** software for any purpose with or without fee is hereby granted.
**
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
** THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
** CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
** FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
** CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
** OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**
***************************************************************/


#include "pf_all.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdarg.h>

static void test_fun(void);

static void pf_dlopen_impl(char* lib_path);
static void pf_dlopen(void);

static void pf_dlclose(void);

static void pf_dlsym_impl(void* lib_ptr, char* func);
static void pf_dlsym(void);

static void* pf_foreign_call_with_return_impl(void* (*func)(void), int arg_count, ...);
static void pf_foreign_call_without_return_impl(void (*func)(void), int arg_count, ...);

static void* pf_foreign_call(void);
static void pf_foreign_call_void(void);

static void call_function(char* lib_path, char* func_name, int arg_count, ...);
static void pf_call_function(void);

static void pf_system_impl(char* sys);
static void pf_system(void);

static void pf_dlopen_impl(char* lib_path) {
    void* handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        free(lib_path);
        return;
    }

    uintptr_t handle_address = (uintptr_t)handle;
    free(lib_path);
    PUSH_DATA_STACK(handle_address);
}

static void pf_dlsym_impl(void* lib_ptr, char* func_name) {
    void* handle = lib_ptr;
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        free(func_name);
        return;
    }

    void* (*func)(void);
    *(void**)(&func) = dlsym(handle, func_name);
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        fprintf(stderr, "%s\n", dlsym_error);
        free(func_name);
        return;
    }

    free(func_name);
    PUSH_DATA_STACK(func);
}



static void* pf_foreign_call_with_return_impl(void* (*func)(void), int arg_count, ...) {
    void* return_val = NULL;
    va_list args;
    va_start(args, arg_count);
    if (arg_count == 0) {
        return_val = ((void* (*)(void))func)();
    } else if (arg_count == 1) {
        return_val = ((void* (*)(void*))func)(va_arg(args, void*));
    } else if (arg_count == 2) {
        return_val = ((void* (*)(void*, void*))func)(va_arg(args, void*), va_arg(args, void*));
    } else if (arg_count == 3) {
        return_val = ((void* (*)(void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*),  va_arg(args, void*));
    } else if (arg_count == 4) {
        return_val = ((void* (*)(void*, void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*));
    } else if (arg_count == 5) {
        return_val = ((void* (*)(void*, void*, void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*));
    }
    // add more as needed, I don't want to write a macro and I don't know the correct way to do this otherwise

    va_end(args);

    return return_val;
}


static void pf_foreign_call_without_return_impl(void (*func)(void), int arg_count, ...) {
    va_list args;
    va_start(args, arg_count);
    if (arg_count == 0) {
        ((void (*)(void))func)();
    } else if (arg_count == 1) {
        ((void (*)(void*))func)(va_arg(args, void*));
    } else if (arg_count == 2) {
        ((void (*)(void*, void*))func)(va_arg(args, void*), va_arg(args, void*));
    } else if (arg_count == 3) {
        ((void (*)(void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*),  va_arg(args, void*));
    } else if (arg_count == 4) {
        ((void (*)(void*, void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*));
    } else if (arg_count == 5) {
        ((void (*)(void*, void*, void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*));
    }
    // add more as needed, I don't want to write a macro and I don't know the correct way to do this otherwise

    va_end(args);

}




// Function to dynamically call any function with arguments
static void call_function(char* lib_path, char* func_name, int arg_count, ...) {
    void* handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        free(lib_path);
        free(func_name);
        return;
    }

    void *(*func)(void);
    *(void**)(&func) = dlsym(handle, func_name);
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        fprintf(stderr, "%s\n", dlsym_error);
        free(lib_path);
        free(func_name);
        dlclose(handle);
        return;
    }

    va_list args;
    va_start(args, arg_count);
    if (arg_count == 0) {
        ((void (*)(void))func)();
    } else if (arg_count == 1) {
        ((void (*)(void*))func)(va_arg(args, void*));
    } else if (arg_count == 2) {
        ((void (*)(void*, void*))func)(va_arg(args, void*), va_arg(args, void*));
    } else if (arg_count == 3) {
        ((void (*)(void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*),  va_arg(args, void*));
    } else if (arg_count == 4) {
        ((void (*)(void*, void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*));
    } else if (arg_count == 5) {
        ((void (*)(void*, void*, void*, void*, void*))func)(va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*), va_arg(args, void*));
    }
    // add more as needed, I don't want to write a macro and I don't know the correct way to do this otherwise

    va_end(args);

    free(lib_path);
    free(func_name);
    dlclose(handle);
}

static void pf_system_impl(char* sys) {
    int result = system(sys);
    free(sys);
    PUSH_DATA_STACK(result);
}

/****************************************************************
** Step 1: Put your own special glue routines here
**     or link them in from another file or library.
****************************************************************/
/* static cell_t CTest0( cell_t Val ) */
/* { */
/*     MSG_NUM_D("CTest0: Val = ", Val); */
/*     return Val+1; */
/* } */

/* static void CTest1( cell_t Val1, cell_t Val2 ) */
/* { */

/*     MSG("CTest1: Val1 = "); ffDot(Val1); */
/*     MSG_NUM_D(", Val2 = ", Val2); */
/* } */

static void pf_system(void) {
    int external_program_name_length = POP_DATA_STACK;
    char* program_name_ptr = (char*)malloc(sizeof(char)*(external_program_name_length + 1));
    const char* program_name = (const char*)POP_DATA_STACK;
    strncpy(program_name_ptr, program_name, external_program_name_length);
    program_name_ptr[external_program_name_length + 1] = ' ';

    pf_system_impl(program_name_ptr);
}

static void* pf_foreign_call(void) {
    int arg_count = (int)POP_DATA_STACK;

    void *(*func)(void);

    *(void**)(&func) = (void*)POP_DATA_STACK;


    // Collect arguments based on arg_count
    void* args[10];
    for (int i = 0; i < arg_count; i++) {
        args[i] = (void*)POP_DATA_STACK;
    }

    return pf_foreign_call_with_return_impl(func, arg_count, args);
}

static void pf_foreign_call_void(void) {
    int arg_count = (int)POP_DATA_STACK;

    void (*vfunc)(void);

    *(void**)(&vfunc) = (void*)POP_DATA_STACK;


    // Collect arguments based on arg_count
    void* args[10];
    for (int i = 0; i < arg_count; i++) {
        args[i] = (void*)POP_DATA_STACK;
    }

    pf_foreign_call_without_return_impl(vfunc, arg_count, args);
}

static void pf_call_function(void) {
    int arg_count = (int)POP_DATA_STACK;
    int func_name_length = (int)POP_DATA_STACK;
    char* func_name_ptr = (char*)malloc(sizeof(char)*(func_name_length + 1));
    const char* func_name = (const char*)POP_DATA_STACK;
    strncpy(func_name_ptr, func_name, func_name_length);
    func_name_ptr[func_name_length + 1] = '\0';

    int lib_path_length = (int)POP_DATA_STACK;
    char* lib_path_ptr = (char*)malloc(sizeof(char)*(lib_path_length + 1));
    const char* lib_path = (const char*)POP_DATA_STACK;

    strncpy(lib_path_ptr, lib_path, lib_path_length);
    lib_path_ptr[lib_path_length + 1] = '\0';

    // Collect arguments based on arg_count
    void* args[10]; // Maximum 10 arguments for simplicity
    for (int i = 0; i < arg_count; i++) {
        args[i] = (void*)POP_DATA_STACK;
    }


    call_function(lib_path_ptr, func_name_ptr, arg_count, args);
    }

static void test_fun(void) {
    int i = (int)POP_DATA_STACK;
    printf("%i", i);
}

static void pf_dlsym(void) {
    int func_name_length = (int)POP_DATA_STACK;
    char* func_name_ptr = (char*)malloc(sizeof(char)*(func_name_length + 1));
    const char* func_name = (const char*)POP_DATA_STACK;
    strncpy(func_name_ptr, func_name, func_name_length);
    func_name_ptr[func_name_length + 1] = '\0';

    void* lib_ptr = (void*)POP_DATA_STACK;

    pf_dlsym_impl(lib_ptr, func_name_ptr);
}


static void pf_dlopen(void) {
    int lib_path_length = (int)POP_DATA_STACK;
    char* lib_path_ptr = (char*)malloc(sizeof(char)*(lib_path_length + 1));
    const char* lib_path = (const char*)POP_DATA_STACK;

    strncpy(lib_path_ptr, lib_path, lib_path_length);
    lib_path_ptr[lib_path_length + 1] = '\0';

    pf_dlopen_impl(lib_path_ptr);
}

static void pf_dlclose(void) {
    void* handle = (void*)POP_DATA_STACK;
    dlclose(handle);
}



/****************************************************************
** Step 2: Create CustomFunctionTable.
**     Do not change the name of CustomFunctionTable!
**     It is used by the pForth kernel.
****************************************************************/

#ifdef PF_NO_GLOBAL_INIT
/******************
** If your loader does not support global initialization, then you
** must define PF_NO_GLOBAL_INIT and provide a function to fill
** the table. Some embedded system loaders require this!
** Do not change the name of LoadCustomFunctionTable()!
** It is called by the pForth kernel.
*/
#define NUM_CUSTOM_FUNCTIONS  (8)
CFunc0 CustomFunctionTable[NUM_CUSTOM_FUNCTIONS];

Err LoadCustomFunctionTable( void )
{
    CustomFunctionTable[0] = pf_call_function;
    CustomFunctionTable[1] = test_fun;
    CustomFunctionTable[2] = pf_foreign_call;
    CustomFunctionTable[3] = pf_foreign_call_void;
    CustomFunctionTable[4] = pf_dlopen;
    CustomFunctionTable[5] = pf_dlclose;
    CustomFunctionTable[6] = pf_dlsym;
    CustomFunctionTable[7] = pf_system;
    return 0;
}

#else
/******************
** If your loader supports global initialization (most do.) then just
** create the table like this.
*/
CFunc0 CustomFunctionTable[] =
{
    (CFunc0) pf_call_function,
    (CFunc0) test_fun,
    (CFunc0) pf_foreign_call,
    (CFunc0) pf_foreign_call_void,
    (CFunc0) pf_dlopen,
    (CFunc0) pf_dlclose,
    (CFunc0) pf_dlsym,
    (CFunc0) pf_system
};
#endif

/****************************************************************
** Step 3: Add custom functions to the dictionary.
**     Do not change the name of CompileCustomFunctions!
**     It is called by the pForth kernel.
****************************************************************/

#if (!defined(PF_NO_INIT)) && (!defined(PF_NO_SHELL))
Err CompileCustomFunctions( void )
{
    Err err;
    int i = 0;
/* Compile Forth words that call your custom functions.
** Make sure order of functions matches that in LoadCustomFunctionTable().
** Parameters are: Name in UPPER CASE, Function, Index, Mode, NumParams
*/
    err = CreateGlueToC( "CALL-FUNCTION", i++, C_RETURNS_VOID, 0 );
    if( err < 0 ) return err;
    err = CreateGlueToC( "TEST-FUN", i++, C_RETURNS_VOID, 0 );
    if( err < 0 ) return err;
    err = CreateGlueToC( "FOREIGN-CALL", i++, C_RETURNS_VALUE, 0 );
    if( err < 0 ) return err;
    err = CreateGlueToC( "FOREIGN-CALL-VOID", i++, C_RETURNS_VOID, 0 );
    if( err < 0 ) return err;
    err = CreateGlueToC( "DLOPEN", i++, C_RETURNS_VOID, 0 );
    if( err < 0 ) return err;
    err = CreateGlueToC( "DLCLOSE", i++, C_RETURNS_VOID, 0 );
    if( err < 0 ) return err;
    err = CreateGlueToC( "DLSYM", i++, C_RETURNS_VOID, 0 );
    if( err < 0 ) return err;
    err = CreateGlueToC( "SYS", i++, C_RETURNS_VOID, 0 );
    if ( err < 0 ) return err;

    return 0;
}
#else
Err CompileCustomFunctions( void ) { return 0; }
#endif

/****************************************************************
** Step 4: Recompile using compiler option PF_USER_CUSTOM
**         and link with your code.
**         Then rebuild the Forth using "pforth -i system.fth"
**         Test:   10 Ctest0 ( should print message then '11' )
****************************************************************/

#endif  /* PF_USER_CUSTOM */

