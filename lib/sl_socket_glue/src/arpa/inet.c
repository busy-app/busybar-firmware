/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * Improved by Marc Boucher <marc@mbsi.ca> and David Haas <dhaas@alum.rpi.edu>
 *
 */

// Modified from original - porta

#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <furi.h>
#include <sys/socket_byteorder.h>

typedef uint32_t u32_t;
typedef int32_t s32_t;
typedef uint8_t u8_t;

typedef struct {
  u32_t addr;
} ip4_addr_t;

typedef struct {
  u32_t addr[4];
} ip6_addr_t;

#define set_errno(err) do { if (err) { errno = (err); } } while(0)

#define lwip_isdigit(d) (d >= '0' && d <= '9')
#define lwip_islower(d) (d >= 'a' && d <= 'z')
#define lwip_isxdigit(d) ((d >= '0' && d <= '9') || (d >= 'a' && d <= 'z') || (d >= 'A' && d <= 'Z'))

static char * ip4addr_ntoa_r(const ip4_addr_t *addr, char *buf, int buflen)
{
  u32_t s_addr;
  char inv[3];
  char *rp;
  u8_t *ap;
  u8_t rem;
  u8_t n;
  u8_t i;
  int len = 0;

  s_addr = addr->addr;

  rp = buf;
  ap = (u8_t *)&s_addr;
  for (n = 0; n < 4; n++) {
    i = 0;
    do {
      rem = *ap % (u8_t)10;
      *ap /= (u8_t)10;
      inv[i++] = (char)('0' + rem);
    } while (*ap);
    while (i--) {
      if (len++ >= buflen) {
        return NULL;
      }
      *rp++ = inv[i];
    }
    if (len++ >= buflen) {
      return NULL;
    }
    *rp++ = '.';
    ap++;
  }
  *--rp = 0;
  return buf;
}

#define ip6_addr_isipv4mappedipv6(ip6addr) (((ip6addr)->addr[0] == 0) && ((ip6addr)->addr[1] == 0) && (((ip6addr)->addr[2]) == htonl(0x0000FFFFUL)))
#define lwip_xchar(i)        ((char)((i) < 10 ? '0' + (i) : 'A' + (i) - 10))

static char *
ip6addr_ntoa_r(const ip6_addr_t *addr, char *buf, int buflen)
{
  u32_t current_block_index, current_block_value, next_block_value;
  s32_t i;
  u8_t zero_flag, empty_block_flag;

  if (ip6_addr_isipv4mappedipv6(addr)) {
    /* This is an IPv4 mapped address */
    ip4_addr_t addr4;
    char *ret;
#define IP4MAPPED_HEADER "::FFFF:"
    char *buf_ip4 = buf + sizeof(IP4MAPPED_HEADER) - 1;
    int buflen_ip4 = buflen - sizeof(IP4MAPPED_HEADER) + 1;
    if (buflen < (int)sizeof(IP4MAPPED_HEADER)) {
      return NULL;
    }
    memcpy(buf, IP4MAPPED_HEADER, sizeof(IP4MAPPED_HEADER));
    addr4.addr = addr->addr[3];
    ret = ip4addr_ntoa_r(&addr4, buf_ip4, buflen_ip4);
    if (ret != buf_ip4) {
      return NULL;
    }
    return buf;
  }
  i = 0;
  empty_block_flag = 0; /* used to indicate a zero chain for "::' */

  for (current_block_index = 0; current_block_index < 8; current_block_index++) {
    /* get the current 16-bit block */
    current_block_value = htonl(addr->addr[current_block_index >> 1]);
    if ((current_block_index & 0x1) == 0) {
      current_block_value = current_block_value >> 16;
    }
    current_block_value &= 0xffff;

    /* Check for empty block. */
    if (current_block_value == 0) {
      if (current_block_index == 7 && empty_block_flag == 1) {
        /* special case, we must render a ':' for the last block. */
        buf[i++] = ':';
        if (i >= buflen) {
          return NULL;
        }
        break;
      }
      if (empty_block_flag == 0) {
        /* generate empty block "::", but only if more than one contiguous zero block,
         * according to current formatting suggestions RFC 5952. */
        next_block_value = htonl(addr->addr[(current_block_index + 1) >> 1]);
        if ((current_block_index & 0x1) == 0x01) {
            next_block_value = next_block_value >> 16;
        }
        next_block_value &= 0xffff;
        if (next_block_value == 0) {
          empty_block_flag = 1;
          buf[i++] = ':';
          if (i >= buflen) {
            return NULL;
          }
          continue; /* move on to next block. */
        }
      } else if (empty_block_flag == 1) {
        /* move on to next block. */
        continue;
      }
    } else if (empty_block_flag == 1) {
      /* Set this flag value so we don't produce multiple empty blocks. */
      empty_block_flag = 2;
    }

    if (current_block_index > 0) {
      buf[i++] = ':';
      if (i >= buflen) {
        return NULL;
      }
    }

    if ((current_block_value & 0xf000) == 0) {
      zero_flag = 1;
    } else {
      buf[i++] = lwip_xchar(((current_block_value & 0xf000) >> 12));
      zero_flag = 0;
      if (i >= buflen) {
        return NULL;
      }
    }

    if (((current_block_value & 0xf00) == 0) && (zero_flag)) {
      /* do nothing */
    } else {
      buf[i++] = lwip_xchar(((current_block_value & 0xf00) >> 8));
      zero_flag = 0;
      if (i >= buflen) {
        return NULL;
      }
    }

    if (((current_block_value & 0xf0) == 0) && (zero_flag)) {
      /* do nothing */
    }
    else {
      buf[i++] = lwip_xchar(((current_block_value & 0xf0) >> 4));
      zero_flag = 0;
      if (i >= buflen) {
        return NULL;
      }
    }

    buf[i++] = lwip_xchar((current_block_value & 0xf));
    if (i >= buflen) {
      return NULL;
    }
  }

  buf[i] = 0;

  return buf;
}

const char *
inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
  const char *ret = NULL;
  int size_int = (int)size;
  if (size_int < 0) {
    set_errno(ENOSPC);
    return NULL;
  }
  switch (af) {
    case AF_INET:
      ret = ip4addr_ntoa_r((const ip4_addr_t *)src, dst, size_int);
      if (ret == NULL) {
        set_errno(ENOSPC);
      }
      break;
    case AF_INET6:
      ret = ip6addr_ntoa_r((const ip6_addr_t *)src, dst, size_int);
      if (ret == NULL) {
        set_errno(ENOSPC);
      }
      break;
    default:
      set_errno(EAFNOSUPPORT);
      break;
  }
  return ret;
}

static int
ip4addr_aton(const char *cp, ip4_addr_t *addr)
{
  u32_t val;
  u8_t base;
  char c;
  u32_t parts[4];
  u32_t *pp = parts;

  c = *cp;
  for (;;) {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, 1-9=decimal.
     */
    if (!lwip_isdigit(c)) {
      return 0;
    }
    val = 0;
    base = 10;
    if (c == '0') {
      c = *++cp;
      if (c == 'x' || c == 'X') {
        base = 16;
        c = *++cp;
      } else {
        base = 8;
      }
    }
    for (;;) {
      if (lwip_isdigit(c)) {
        if((base == 8) && ((u32_t)(c - '0') >= 8))
          break;
        val = (val * base) + (u32_t)(c - '0');
        c = *++cp;
      } else if (base == 16 && lwip_isxdigit(c)) {
        val = (val << 4) | (u32_t)(c + 10 - (lwip_islower(c) ? 'a' : 'A'));
        c = *++cp;
      } else {
        break;
      }
    }
    if (c == '.') {
      /*
       * Internet format:
       *  a.b.c.d
       *  a.b.c   (with c treated as 16 bits)
       *  a.b (with b treated as 24 bits)
       */
      if (pp >= parts + 3) {
        return 0;
      }
      *pp++ = val;
      c = *++cp;
    } else {
      break;
    }
  }
  /*
   * Check for trailing characters.
   */
  if (c != '\0' && !isspace(c)) {
    return 0;
  }
  /*
   * Concoct the address according to
   * the number of parts specified.
   */
  switch (pp - parts + 1) {

    case 0:
      return 0;       /* initial nondigit */

    case 1:             /* a -- 32 bits */
      break;

    case 2:             /* a.b -- 8.24 bits */
      if (val > 0xffffffUL) {
        return 0;
      }
      if (parts[0] > 0xff) {
        return 0;
      }
      val |= parts[0] << 24;
      break;

    case 3:             /* a.b.c -- 8.8.16 bits */
      if (val > 0xffff) {
        return 0;
      }
      if ((parts[0] > 0xff) || (parts[1] > 0xff)) {
        return 0;
      }
      val |= (parts[0] << 24) | (parts[1] << 16);
      break;

    case 4:             /* a.b.c.d -- 8.8.8.8 bits */
      if (val > 0xff) {
        return 0;
      }
      if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff)) {
        return 0;
      }
      val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
      break;
    default:
      furi_crash();
      break;
  }
  if (addr) {
    addr->addr = htonl(val);
  }
  return 1;
}

static int
ip6addr_aton(const char *cp, ip6_addr_t *addr)
{
  u32_t addr_index, zero_blocks, current_block_index, current_block_value;
  const char *s;
  int check_ipv4_mapped = 0;

  /* Count the number of colons, to count the number of blocks in a "::" sequence
     zero_blocks may be 1 even if there are no :: sequences */
  zero_blocks = 8;
  for (s = cp; *s != 0; s++) {
    if (*s == ':') {
      zero_blocks--;
    } else if (*s == '.') {
      if ((zero_blocks == 5) ||(zero_blocks == 2)) {
        check_ipv4_mapped = 1;
        /* last block could be the start of an IPv4 address */
        zero_blocks--;
      } else {
        /* invalid format */
        return 0;
      }
      break;
    } else if (!lwip_isxdigit(*s)) {
      break;
    }
  }

  /* parse each block */
  addr_index = 0;
  current_block_index = 0;
  current_block_value = 0;
  for (s = cp; *s != 0; s++) {
    if (*s == ':') {
      if (addr) {
        if (current_block_index & 0x1) {
          addr->addr[addr_index++] |= current_block_value;
        }
        else {
          addr->addr[addr_index] = current_block_value << 16;
        }
      }
      current_block_index++;
      if (check_ipv4_mapped) {
        if (current_block_index == 6) {
          ip4_addr_t ip4;
          int ret = ip4addr_aton(s + 1, &ip4);
          if (ret) {
            if (addr) {
              addr->addr[3] = htonl(ip4.addr);
              current_block_index++;
              goto fix_byte_order_and_return;
            }
            return 1;
          }
        }
      }
      current_block_value = 0;
      if (current_block_index > 7) {
        /* address too long! */
        return 0;
      }
      if (s[1] == ':') {
        if (s[2] == ':') {
          /* invalid format: three successive colons */
          return 0;
        }
        s++;
        /* "::" found, set zeros */
        while (zero_blocks > 0) {
          zero_blocks--;
          if (current_block_index & 0x1) {
            addr_index++;
          } else {
            if (addr) {
              addr->addr[addr_index] = 0;
            }
          }
          current_block_index++;
          if (current_block_index > 7) {
            /* address too long! */
            return 0;
          }
        }
      }
    } else if (lwip_isxdigit(*s)) {
      /* add current digit */
      current_block_value = (current_block_value << 4) +
          (lwip_isdigit(*s) ? (u32_t)(*s - '0') :
          (u32_t)(10 + (lwip_islower(*s) ? *s - 'a' : *s - 'A')));
    } else {
      /* unexpected digit, space? CRLF? */
      break;
    }
  }

  if (addr) {
    if (current_block_index & 0x1) {
      addr->addr[addr_index++] |= current_block_value;
    }
    else {
      addr->addr[addr_index] = current_block_value << 16;
    }
fix_byte_order_and_return:
    /* convert to network byte order. */
    for (addr_index = 0; addr_index < 4; addr_index++) {
      addr->addr[addr_index] = htonl(addr->addr[addr_index]);
    }
  }

  if (current_block_index != 7) {
    return 0;
  }

  return 1;
}

int
inet_pton(int af, const char *src, void *dst)
{
  int err;
  switch (af) {
    case AF_INET:
      err = ip4addr_aton(src, (ip4_addr_t *)dst);
      break;
    case AF_INET6: {
      /* convert into temporary variable since ip6_addr_t might be larger
         than in6_addr when scopes are enabled */
      ip6_addr_t addr;
      err = ip6addr_aton(src, &addr);
      if (err) {
        memcpy(dst, &addr.addr, sizeof(addr.addr));
      }
      break;
    }
    default:
      err = -1;
      set_errno(EAFNOSUPPORT);
      break;
  }
  return err;
}
