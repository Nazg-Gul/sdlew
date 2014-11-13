/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2013 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/**
 *  \file SDL_rwops.h
 *
 *  This file provides a general interface for SDL to read and write
 *  data streams.  It can easily be extended to files, memory, etc.
 */

#ifndef _SDL_rwops_h
#define _SDL_rwops_h

#include "SDL_stdinc.h"
#include "SDL_error.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* RWops Types */
#define SDL_RWOPS_UNKNOWN   0   /* Unknown stream type */
#define SDL_RWOPS_WINFILE   1   /* Win32 file */
#define SDL_RWOPS_STDFILE   2   /* Stdio file */
#define SDL_RWOPS_JNIFILE   3   /* Android asset */
#define SDL_RWOPS_MEMORY    4   /* Memory stream */
#define SDL_RWOPS_MEMORY_RO 5   /* Read-Only memory stream */

/**
 * This is the read/write operation structure -- very basic.
 */
typedef struct SDL_RWops
{
    /**
     *  Return the size of the file in this rwops, or -1 if unknown
     */
    Sint64 (SDLCALL * size) (struct SDL_RWops * context);

    /**
     *  Seek to \c offset relative to \c whence, one of stdio's whence values:
     *  RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END
     *
     *  \return the final offset in the data stream, or -1 on error.
     */
    Sint64 (SDLCALL * seek) (struct SDL_RWops * context, Sint64 offset,
                             int whence);

    /**
     *  Read up to \c maxnum objects each of size \c size from the data
     *  stream to the area pointed at by \c ptr.
     *
     *  \return the number of objects read, or 0 at error or end of file.
     */
    size_t (SDLCALL * read) (struct SDL_RWops * context, void *ptr,
                             size_t size, size_t maxnum);

    /**
     *  Write exactly \c num objects each of size \c size from the area
     *  pointed at by \c ptr to data stream.
     *
     *  \return the number of objects written, or 0 at error or end of file.
     */
    size_t (SDLCALL * write) (struct SDL_RWops * context, const void *ptr,
                              size_t size, size_t num);

    /**
     *  Close and free an allocated SDL_RWops structure.
     *
     *  \return 0 if successful or -1 on write error when flushing data.
     */
    int (SDLCALL * close) (struct SDL_RWops * context);

    Uint32 type;
    union
    {
#if defined(ANDROID)
        struct
        {
            void *fileNameRef;
            void *inputStreamRef;
            void *readableByteChannelRef;
            void *readMethod;
            void *assetFileDescriptorRef;
            long position;
            long size;
            long offset;
            int fd;
        } androidio;
#elif defined(__WIN32__)
        struct
        {
            SDL_bool append;
            void *h;
            struct
            {
                void *data;
                size_t size;
                size_t left;
            } buffer;
        } windowsio;
#endif

#ifdef HAVE_STDIO_H
        struct
        {
            SDL_bool autoclose;
            FILE *fp;
        } stdio;
#endif
        struct
        {
            Uint8 *base;
            Uint8 *here;
            Uint8 *stop;
        } mem;
        struct
        {
            void *data1;
            void *data2;
        } unknown;
    } hidden;

} SDL_RWops;


/**
 *  \name RWFrom functions
 *
 *  Functions to create SDL_RWops structures from various data streams.
 */
/*@{*/

typedef SDL_RWops * SDLCALL tSDL_RWFromFile(const char *file,
                                                  const char *mode);

#ifdef HAVE_STDIO_H
typedef SDL_RWops * SDLCALL tSDL_RWFromFP(FILE * fp,
                                                SDL_bool autoclose);
#else
typedef SDL_RWops * SDLCALL tSDL_RWFromFP(void * fp,
                                                SDL_bool autoclose);
#endif

typedef SDL_RWops * SDLCALL tSDL_RWFromMem(void *mem, int size);
typedef SDL_RWops * SDLCALL tSDL_RWFromConstMem(const void *mem,
                                                      int size);

/*@}*//*RWFrom functions*/


typedef SDL_RWops * SDLCALL tSDL_AllocRW(void);
typedef void SDLCALL tSDL_FreeRW(SDL_RWops * area);

#define RW_SEEK_SET 0       /**< Seek from the beginning of data */
#define RW_SEEK_CUR 1       /**< Seek relative to current read point */
#define RW_SEEK_END 2       /**< Seek relative to the end of data */

/**
 *  \name Read/write macros
 *
 *  Macros to easily read and write from an SDL_RWops structure.
 */
/*@{*/
#define SDL_RWsize(ctx)         (ctx)->size(ctx)
#define SDL_RWseek(ctx, offset, whence) (ctx)->seek(ctx, offset, whence)
#define SDL_RWtell(ctx)         (ctx)->seek(ctx, 0, RW_SEEK_CUR)
#define SDL_RWread(ctx, ptr, size, n)   (ctx)->read(ctx, ptr, size, n)
#define SDL_RWwrite(ctx, ptr, size, n)  (ctx)->write(ctx, ptr, size, n)
#define SDL_RWclose(ctx)        (ctx)->close(ctx)
/*@}*//*Read/write macros*/


/**
 *  \name Read endian functions
 *
 *  Read an item of the specified endianness and return in native format.
 */
/*@{*/
typedef Uint8 SDLCALL tSDL_ReadU8(SDL_RWops * src);
typedef Uint16 SDLCALL tSDL_ReadLE16(SDL_RWops * src);
typedef Uint16 SDLCALL tSDL_ReadBE16(SDL_RWops * src);
typedef Uint32 SDLCALL tSDL_ReadLE32(SDL_RWops * src);
typedef Uint32 SDLCALL tSDL_ReadBE32(SDL_RWops * src);
typedef Uint64 SDLCALL tSDL_ReadLE64(SDL_RWops * src);
typedef Uint64 SDLCALL tSDL_ReadBE64(SDL_RWops * src);
/*@}*//*Read endian functions*/

/**
 *  \name Write endian functions
 *
 *  Write an item of native format to the specified endianness.
 */
/*@{*/
typedef size_t SDLCALL tSDL_WriteU8(SDL_RWops * dst, Uint8 value);
typedef size_t SDLCALL tSDL_WriteLE16(SDL_RWops * dst, Uint16 value);
typedef size_t SDLCALL tSDL_WriteBE16(SDL_RWops * dst, Uint16 value);
typedef size_t SDLCALL tSDL_WriteLE32(SDL_RWops * dst, Uint32 value);
typedef size_t SDLCALL tSDL_WriteBE32(SDL_RWops * dst, Uint32 value);
typedef size_t SDLCALL tSDL_WriteLE64(SDL_RWops * dst, Uint64 value);
typedef size_t SDLCALL tSDL_WriteBE64(SDL_RWops * dst, Uint64 value);
/*@}*//*Write endian functions*/


extern tSDL_RWFromFile *SDL_RWFromFile;
extern tSDL_RWFromFP *SDL_RWFromFP;
extern tSDL_RWFromFP *SDL_RWFromFP;
extern tSDL_RWFromMem *SDL_RWFromMem;
extern tSDL_RWFromConstMem *SDL_RWFromConstMem;
extern tSDL_AllocRW *SDL_AllocRW;
extern tSDL_FreeRW *SDL_FreeRW;
extern tSDL_ReadU8 *SDL_ReadU8;
extern tSDL_ReadLE16 *SDL_ReadLE16;
extern tSDL_ReadBE16 *SDL_ReadBE16;
extern tSDL_ReadLE32 *SDL_ReadLE32;
extern tSDL_ReadBE32 *SDL_ReadBE32;
extern tSDL_ReadLE64 *SDL_ReadLE64;
extern tSDL_ReadBE64 *SDL_ReadBE64;
extern tSDL_WriteU8 *SDL_WriteU8;
extern tSDL_WriteLE16 *SDL_WriteLE16;
extern tSDL_WriteBE16 *SDL_WriteBE16;
extern tSDL_WriteLE32 *SDL_WriteLE32;
extern tSDL_WriteBE32 *SDL_WriteBE32;
extern tSDL_WriteLE64 *SDL_WriteLE64;
extern tSDL_WriteBE64 *SDL_WriteBE64;

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_rwops_h */

/* vi: set ts=4 sw=4 expandtab: */
