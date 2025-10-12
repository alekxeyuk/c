#ifndef TORRENT_FILE_H_
#define TORRENT_FILE_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define HASH_SIZE 22
#define NAME_MAX 255

typedef struct {
  // Хэш сумма самого torrent файла. Мы используем его для поиска сидов/пиров в сети имеющих данный торрент файл и файл ему соответствующий
  u_char infohash[HASH_SIZE + 1];
  // Размер файла в байтах
  uint32_t length;
  // Размер 1 фрагмента в байтах (64кб по ТЗ)
  uint32_t piece_length;
  // Имя файла
  u_char name[NAME_MAX + 1];
  // массив байтов кратный HASH_SIZE (n = HASH_SIZE * ceil(length/piece_length)), состоит из хэшей каждого из фрагментов
  u_char* pieces;
} __attribute__((packed)) eltextorrent_file_t;

#endif // TORRENT_FILE_H_