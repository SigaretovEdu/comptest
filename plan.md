# Plan for comptest 2.0

1. Write in C/C++

## Features

comptest source test [-c,--checker checker] [-t,--test
+-[NUM][NUM,...,NUM][NUM..NUM]] [-q, --quite] [-d, --disable-test]

1. Can use .c .cpp .py files for source and checker
2. Can specify test with -t/--test. If NUM > 0, then use only this test,
   otherwise exclude test. NUM can be write as "NUM" for one test, "NUM,...,NUM"
   for a list of tests, "NUM..NUM" for range
3. Can specify quite output, for each test there will be only result
4. Can specify -d/--disable-test to disable comparsion of output and output from
   test
5. Can specify TL in testfile
6. Use custom diff

Output should be like:

```
---- test_num WA/TL/RE/UN
input
----
output
----

ok(num) failed(num) unknown(num)
```

For --quite

```
test_num OK/WA/TL/RE/UN
...
test_num OK/WA/TL/RE/UN

ok(num) failed(num) unknown(num)
```

Testfile:

```
TL = 3

>> [test_num]
input
>>
output
<<

>>
input
<<
```
