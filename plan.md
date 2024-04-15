# Plan for comptest 2.0

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
TL = 2

-- 1
1
2
-- out
1
2
14
15
--

-- 2
9
10
-- out
9
10
--
```

```
Usage: comptest [OPTIONS...] sourcefile testsfiles...

    -t, --test testfile:[NUM][NUM,...,NUM][NUM..NUM]    specify tests to use from file, [NUM] will use the only test,
                                                        [NUM,...,NUM] will use tests in specified list, [NUM..NUM]
                                                        will use test in range

    -c, --compare sourcefile                            specify file with another solution, every test will be passed
                                                        to your solution and this file, outputs will be compared

    -q, --quite                                         do not show the outputs of test, only result for every test

    -d, --disable                                       disable comparing output from your solution with
                                                        specified in test file

    -h, --help                                          show this message

```

Example output:

```
TL = 2

-- test_3 WA
diff
--

-- test_7 WA
diff
--

-- test_9 WA
diff
--

[OK] 5
[WA] 3
```

--quite

```
TL = 2
[OK] 5: 1, 2, 3, 4, 5
[WA] 3: 6, 7, 8
```
