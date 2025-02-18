# espArchive

Набор инструментов для работы с архивом формата .ar на esp
 - загрузка архива .ar в FS ESP
 - работа с архивом, как с файловой системой ReadOnly
  - доступ к файлу по хешу или по имени
 - загрузка архива .ar в ESP с разархивацией файлов "на лету" в FS ESP

# Зачем?

При работе с esp часто приходится иметь дело со статическим контентом ( страницы html, скрипты js и т.п.). Контент может быть жестко закодирован в коде или храниться в файловой системе esp. Второй вариант предпочтительный, но возникает проблема с обновлением страниц при необходимости. Можно обновить целиком всю файловую систему прошивкой по OTA, но тогда потеряются файлы настроек. 
Можно грузить весь контент поочердно, но установление соединения занимает довольно много времени на esp, особенно по https с проверкой сертификатов.

Данная библиотека позволяет собрать весь контент в один архив .ar (на линуксе архиватор присутствует в базовой поставке) и загружать только один архив со множеством файлов.
Возможна разархивация "на лету" с записью в файловую систему esp или загрузка архива с последующим доступом в него как к папке ( в будущем ). 

Пока реализован доступ к файлам в архиве без наследования типа File.

## Документация

в разработке. См. Примеры.

### Примеры

#### ArFileDownloader.ino
Загрузка архива по https с разархивацией "на лету" в файловую систему LittleFS.
В архиве два файла. После загрузки показывает их содержимое.

#### ArStream.ino
Загрузка архива по https в ФС esp и работа с файлами в архиве. 
Доступ к файлам по имени или по хешу имени.
Показывает содержимое файлов.

#### EspArFs.ino
***Внимание!!*** *Запускать строго после ArStream.ino.*

Доступ к файлам в имеющемся архиве по имени или по хешу имени.
Показывает содержимое файлов.

## ToDo

Наследовать тип Dir файловой системы к архиву .ar и тип File к содержимому архивов.
Сделать совместимой с esp32.