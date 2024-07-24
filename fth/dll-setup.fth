\ assumes existence of ../hello.cpp, outputting to hello.so

: c-string swap over pad dup >r swap move r@ over + 0 swap c! r> swap 1 + ;

: Z" ['] S" execute c-string ;

\ assumes that there is room after the original string location
: strconcat ( original-c-address original-len added-c-address added-len -- original-c-address new-len )
  >r over r@ + r> swap >r >r rot >r r@ r> r> swap >r >r rot + r> move r> r> ;

\ ex. compile-cpp ./hello.so ../hello.cpp
\ ex2. compile-cpp ./hello.so ../*.cpp
: compile-cpp ( "output-file" "input-file" -- )
  S" g++ -shared -fPIC -o " pad swap dup >r 0 DO over i + over i + swap c@ swap c! LOOP SWAP DROP r> \ copy g++ raw string to pad location ( pad-address len )
  BL PARSE strconcat \ take the next word from TIB and use it as the output-file with -o
  S"  " strconcat \ add a space to the command
  S" cpp-src/*.cpp cpp-src/*.a" strconcat \ add next word from TIB to the end of the output name string to be used as input
  c-string sys throw ; \ turn the whole thing into a c-style string and call C's system function on it


S" ./hello.so" c-string dlopen

value hello-lib

hello-lib Z" say_hello" dlsym

value say-hello



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

Z" ./hello3.so" dlopen

value hello3

hello3 Z" return_hello" dlsym

value return-hello
