\ assumes existence of ../hello.cpp, outputting to hello.so

: c-string swap over here swap move here over + 0 swap c! here swap 1 + ;

: Z" ['] S" execute c-string ;

: recompile-hello S" g++ -shared -fPIC -o hello.so ../hello.cpp" c-string sys throw ;

S" ./hello.so" c-string dlopen

variable hello-lib

hello-lib !

hello-lib @ Z" say_hello" dlsym

variable say-hello

say-hello !

\ addr len
\ len addr \ swap
\ len addr len \ over
\ len addr len here \ here
 \ len addr here len \ swap
\ move -> len \ move
\ len here \ here
\ len here len \ over
\ len here+len 0 \ + 0
\ len 0 here+len \ swap
\ len \ c!
\ len here \ here
\ here len \ swap
\ here len+1 \ 1 +

\ c-fun-addr arg-count is-void-return foreign-call
\ say_hello @ 0 true foreign-call
