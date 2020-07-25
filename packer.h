//
// Created by matthew on 20/07/2020.
//

#ifndef DATABASE_MANAGER_PACKER_H
#define DATABASE_MANAGER_PACKER_H

#ifdef __GNUC__
// #define PACK(__Declaration__) __Declaration__ __attribute__((packed));
#define PACK_START
#define PACK_END __attribute__((packed));
#endif

#ifdef _MSC_VER
// #define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__; __pragma(pack(pop))
#define PACK_START __pragma(pack(push, 1))
#define PACK_END ; __pragma(pack(pop));
#endif

#endif //DATABASE_MANAGER_PACKER_H