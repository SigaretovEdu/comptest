# comptest

Небольшой скрипт для тестирования решения

## Зависимости:
* python
* icdiff

## Использование:
```
usage: testing.py [-h] [-c CHECKER] [-t TEST] source test_file

comptest: Compile and test

positional arguments:
  source                source code
  test_file             file with test

options:
  -h, --help            show this help message and exit
  -c CHECKER, --checker CHECKER
                        file with right code
  -t TEST, --test TEST  test num
```

```-c FILEPATH``` указывает файл относительно ответов которого будет тестироваться ваше решение

```-t INT``` использовать только один тест из файла тестов

## Формат файла с тестами:
```
>> 1
test1 input
>>
test1 output
<<

>> 2
test2 input
<<

>>
test3 input
>>
test3 output
<<
```
На примере выше показаны три теста, у каждого теста может быть указан правильный output, опционально можно укзаать номер теста, однако лучше соблюдать порядок по увеличению и не указывать одинаковые номера

Для тестов без правильного ouput будет выставлено значение UNKNOWN

## Настройка:
В скрипте можно для необходимых языков указать опции компиляции, формат запуска, лимит по времени для выполнения программы в секундах

```
compile_options = {
    "cpp": {
        "compiler": "g++",
        "options": ["-Wall", "-Wextra", "-Wpedantic", "-o"]
    },
    "c": {
        "compiler": "gcc",
        "options": ["-Wall", "-Wextra", "-Wpedantic", "-o"]
    },
}

run_options = {
    "py": "python"
}

# TL in seconds
TL = 1
```