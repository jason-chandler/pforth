1 value lib-number
0 value dev-lib

: string-to-pad swap over pad dup >r swap cmove r> swap ; 

: string-to-buffer ( c-address c-len -- ) 
  dup >r S" marker strbuffer" evaluate dup chars here dup >r swap allot swap cmove r> r> ; 

: c-string swap over pad dup >r swap cmove r@ over + 0 swap c! r> swap 1 + ;

: Z" ['] S" execute c-string ;

: number-to-string   s>d <# #s #> ;

\ assumes that there is room after the original string location
: strconcat ( original-c-address original-len added-c-address added-len -- original-c-address new-len )
  >r over r@ + r> swap >r >r rot >r r@ r> r> swap >r >r rot + r> cmove r> r> ;

: nvim ( c-address c-len -- )
  string-to-pad >r >r S" nvim " string-to-buffer r> r> strconcat c-string sys drop S" strbuffer" evaluate ; 

: conditionally-dlclose dev-lib 0= not if dev-lib dlclose then ;

\ ex. compile-cpp ./hello.so ../hello.cpp
\ ex2. compile-cpp ./hello.so ../*.cpp
: compile-cpp ( -- )
  conditionally-dlclose
  S" g++ -std=c++17 -L./forth-extension -Wl,-rpath,./forth-extension -shared -g -fPIC -o " 
  pad swap dup >r 0 DO over i + over i + swap c@ swap c! LOOP SWAP DROP r> \ copy g++ raw string to pad location ( pad-address len )
  S" G" strconcat \ take the next word from TIB and use it as the output-file with -o
  lib-number number-to-string string-to-buffer strconcat S" strbuffer" evaluate 
  S"  -I./forth-extension -isystem ./forth-extension -fpermissive " strconcat
  \ add a # until we reach a lib name we haven't used yet
  S" ./forth-extension/*.cpp ./forth-extension/*.a" strconcat 
  \ add next word from TIB to the end of the output name string to be used as input
  c-string sys throw pad 84 chars + 1 lib-number begin swap 1+ swap 10 / dup 0 = until
  drop c-string over over dlopen TO dev-lib delete-file throw
  lib-number 1+ to lib-number ; \ turn the whole thing into a c-style string and call C's system function on it

: ~cpp S" ./forth-extension/" string-to-buffer BL parse strconcat
       w/o create-file throw
       begin >r 126 parse dup r> swap >r >r r@
             write-file throw r> r> dup
             if over S\" \n" rot write-file throw cr refill drop then
             0= until  close-file throw S" strbuffer" evaluate ;



\ S" ./hello.so" c-string dlopen

\ value hello-lib

\ hello-lib Z" say_hello" dlsym

\ value say-hello



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

\ Z" ./hello3.so" dlopen

\ value hello3

\ hello3 Z" return_hello" dlsym

\ value return-hello
