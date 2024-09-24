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


/// \def PACK_START
/// Tells the compiler to save the state of packing, then start densely packing members of structures, with no padding between.

/*!
    \brief Tells the compiler to save the state of packing, then start densely packing members of structures, with no padding between.
*/
#define PACK_START __pragma(pack(push, 1))
/// \def PACK_END
/// Tells the compiler to stop packing densely, and return to the previously saved state of packing.
#define PACK_END ; __pragma(pack(pop));
#endif

#endif //DATABASE_MANAGER_PACKER_H