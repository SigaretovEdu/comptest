# comptest 2.0

# Desciption

This is just a simple script to run and chek testcase in competetive programming

# Dependencies

* icdiff

# Usage

```
Usage: comptest [OPTIONS...] sourcefile testsfiles...

    -t, --test [-][NUM][NUM,...,NUM][NUM..NUM]          specify tests to use from file, [NUM] will use
                                                        the only test,[NUM,...,NUM] will use tests in
                                                        specified list, [NUM..NUM] will use test in
                                                        range

    -c, --compare sourcefile                            specify file with another solution, every test
                                                        will be  passed to your solution and this file,
                                                        outputs will be compared

    -q, --quite                                         do not show the outputs of test, only summary

    -d, --disable                                       disable comparing output from your solution
                                                        with specified in test file

    -h, --help                                          show this message

```

For example: you have your solution code in file sol.c and a bunch of testcases in two files small.test and big.test

Then you can run your testcases like this

```
comptest sol.c small.test big.test
```

Output should be something like that:

```
[WA] 2
-- input
9
10
-- output
9                           9
10                          10
                            1
                            2
--


[OK] total(1) : 1
[WA] total(1) : 2
```

Where you can see every testcase, that failed, and for this cases your output and expected output, compared by icdiff. Below detailed output of failed testcases, you can see summary information by all cases.

`-t` or `--test` option allow you to scpecify what test to use. You can provide ids as single number `1`,  list `1,2,5`, or range `1..10`. With `-` at the begining you will exclude specified tests from checking. For example `-1..3` will exclude tests with ids 1, 2, 3

`-c` or `--compare` option allow you to specify another solution, for every testcase outputs from your solution and specified by this option will be compared

`-q` or `--quite` option allow you to disable output of failed testcases and print only summary

`-d` or `--disable` will turn of comparing of your output and expected and all testcases will be marked as UNKNOWN

# Results

`OK` - your solution provided same output as expected

`WA` - your solution provided output that is different from expected

`ER` - your solution failed during execution

`TL` - your solution worked for too long and hit time limit

`UK` - there is no expected output for this case, so result is UNKNOWN

# `.test` file format

Basic testfile can look like this

```
TL = 2

-- <test id>
<input>
-- out
<expected output>
--

-- <test id>
<input>
-- out
<expected output>
-- out
<another possible output>
--

-- <test id>
<input>
--

...
```

For test id you must provide unique integer. You can specify as many possible outputs as you want, or not even provide.

You can provide as many testfiles as you want, but test ids should be a unique number between all files.

You can specify timelimit in seconds for testcases.

# Add compilation details
