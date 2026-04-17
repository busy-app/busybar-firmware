/*******************************************************************************
 * @file
 * @brief Stub implementation of the tcp interface.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <openthread/instance.h>
#include <openthread/message.h>

#include "bsdtcp/tcp_var.h"
#include "lib/cbuf.h"

void bmp_init(uint8_t *buf, size_t numbytes)
{
    OT_UNUSED_VARIABLE(buf);
    OT_UNUSED_VARIABLE(numbytes);
}

size_t cbuf_free_space(struct cbufhead *chdr)
{
    OT_UNUSED_VARIABLE(chdr);
    return 0;
}

void cbuf_init(struct cbufhead *chdr, uint8_t *buf, size_t len)
{
    OT_UNUSED_VARIABLE(chdr);
    OT_UNUSED_VARIABLE(buf);
    OT_UNUSED_VARIABLE(len);
}

size_t cbuf_pop(struct cbufhead *chdr, size_t numbytes)
{
    OT_UNUSED_VARIABLE(chdr);
    OT_UNUSED_VARIABLE(numbytes);
    return 0;
}

void cbuf_reference(const struct cbufhead *chdr, struct otLinkedBuffer *first, struct otLinkedBuffer *second)
{
    OT_UNUSED_VARIABLE(chdr);
    OT_UNUSED_VARIABLE(first);
    OT_UNUSED_VARIABLE(second);
}

void cbuf_contiguify(struct cbufhead *chdr, uint8_t *bitmap)
{
    OT_UNUSED_VARIABLE(chdr);
    OT_UNUSED_VARIABLE(bitmap);
}

size_t cbuf_used_space(struct cbufhead *chdr)
{
    OT_UNUSED_VARIABLE(chdr);
    return 0;
}

void initialize_tcb(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
}

void lbuf_init(struct lbufhead *buffer)
{
    OT_UNUSED_VARIABLE(buffer);
}

size_t lbuf_used_space(const struct lbufhead *buffer)
{
    OT_UNUSED_VARIABLE(buffer);
    return 0;
}

int tcp6_usr_connect(struct tcpcb *tp, struct sockaddr_in6 *sinp6)
{
    OT_UNUSED_VARIABLE(tp);
    OT_UNUSED_VARIABLE(sinp6);
    return ENOSYS;
}

void tcp_dropwithreset(struct ip6_hdr *ip6,
                       struct tcphdr  *th,
                       struct tcpcb   *tp,
                       otInstance     *instance,
                       int             tlen,
                       int             rstreason)
{
    OT_UNUSED_VARIABLE(ip6);
    OT_UNUSED_VARIABLE(th);
    OT_UNUSED_VARIABLE(tp);
    OT_UNUSED_VARIABLE(instance);
    OT_UNUSED_VARIABLE(tlen);
    OT_UNUSED_VARIABLE(rstreason);
}

int tcplp_input(struct ip6_hdr       *ip6,
                struct tcphdr        *th,
                otMessage            *msg,
                struct tcpcb         *tp,
                struct tcpcb_listen  *tpl,
                struct tcplp_signals *sig)
{
    OT_UNUSED_VARIABLE(ip6);
    OT_UNUSED_VARIABLE(th);
    OT_UNUSED_VARIABLE(msg);
    OT_UNUSED_VARIABLE(tp);
    OT_UNUSED_VARIABLE(tpl);
    OT_UNUSED_VARIABLE(sig);
    return ENOSYS;
}

int tcp_timer_2msl(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
    return ENOSYS;
}

int tcp_timer_active(struct tcpcb *tp, uint32_t timer_type)
{
    OT_UNUSED_VARIABLE(tp);
    OT_UNUSED_VARIABLE(timer_type);
    return ENOSYS;
}

int tcp_timer_delack(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
    return ENOSYS;
}

int tcp_timer_keep(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
    return ENOSYS;
}

int tcp_timer_persist(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
    return ENOSYS;
}

int tcp_timer_rexmt(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
    return ENOSYS;
}

void tcp_usr_abort(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
}

int tcp_usr_rcvd(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
    return ENOSYS;
}

int tcp_usr_send(struct tcpcb          *tp,
                 int                    moretocome,
                 struct otLinkedBuffer *data,
                 size_t                 extendby,
                 struct sockaddr_in6   *nam)
{
    OT_UNUSED_VARIABLE(tp);
    OT_UNUSED_VARIABLE(moretocome);
    OT_UNUSED_VARIABLE(data);
    OT_UNUSED_VARIABLE(extendby);
    OT_UNUSED_VARIABLE(nam);
    return ENOSYS;
}

int tcp_usr_shutdown(struct tcpcb *tp)
{
    OT_UNUSED_VARIABLE(tp);
    return ENOSYS;
}
