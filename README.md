# ДЗ-11. Outer join

## Проверка в Docker

Для проверки достаточно установленного и запущенного Docker. Из корня проекта
выполнить:

```bash
./prj build
./prj test
```

Скрипты автоматически создадут образ `otus_builder`, соберут проект и запустят
модульные и сетевые тесты. Все тесты должны завершиться успешно.

Покрытие production-кода можно проверить отдельно:

```bash
./prj cov
```

HTML-отчёт будет создан в `build/out/index.html`.

## Ручная проверка сервера в Docker

После сборки запустить сервер в первом терминале:

```bash
docker run --rm --init \
  -v "$PWD:/app" \
  -w /app \
  -p 9000:9000 \
  otus_builder \
  ./build/bin/join_server 9000
```

Во втором терминале подключиться к серверу:

```bash
nc 127.0.0.1 9000
```

Пример команд и ожидаемых ответов:

```text
INSERT A 3 violation
OK
INSERT B 3 proposal
OK
INTERSECTION
3,violation,proposal
OK
SYMMETRIC_DIFFERENCE
OK
```

Сервер завершается сочетанием `Ctrl+C` в первом терминале.

## Локальная проверка без Docker

Для Ubuntu установить зависимости:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  libboost-system-dev \
  libsqlite3-dev \
  netcat-openbsd
```

Собрать проект и запустить тесты:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Для ручной проверки запустить сервер:

```bash
./build/bin/join_server 9000
```

Затем подключиться к нему из второго терминала командой
`nc 127.0.0.1 9000` и выполнить сценарий выше.

## Проверка DEB-пакета

После сборки выполнить:

```bash
cmake --build build --target package
```

В каталоге `build` должен появиться пакет
`join_server-0.0.1-Linux.deb`. Проверить его содержимое можно командой:

```bash
dpkg-deb --contents build/join_server-0.0.1-Linux.deb
```

Пакет должен содержать исполняемый файл `/usr/bin/join_server`.
