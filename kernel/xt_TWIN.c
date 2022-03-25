/*
 * TCP window modification target for IP tables
 * (C) 2015 by Sergej Pupykin <sergej@p5n.pp.ru>
 * (C) 2017 fixes by Vadim Fedorenko <junjunk@fromru.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <linux/netfilter/x_tables.h>
#include "ipt_TWIN.h"

MODULE_AUTHOR("Harald Welte <laforge@netfilter.org>");
MODULE_AUTHOR("Vadim Fedorenko <junjunk@fromru.com>");
MODULE_DESCRIPTION("Xtables: TCPWIN field modification target");
MODULE_LICENSE("GPL");

static inline unsigned int
optlen(const u_int8_t *opt, unsigned int offset)
{
	/* Beware zero-length options: make finite progress */
	if (opt[offset] <= TCPOPT_NOP || opt[offset+1] == 0)
		return 1;
	else
		return opt[offset+1];
}

static unsigned int
twin_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	struct tcphdr *tcph;
	struct iphdr *iph;
	const struct ipt_TWIN_info *info = par->targinfo;
	int offset, len;
    int i;
	int tcp_hdrlen;
	u_int8_t *opt;

	if (skb_ensure_writable(skb, skb->len))
		return NF_DROP;
	if (skb_linearize(skb))
		return NF_DROP;
	iph = ip_hdr(skb);
	if (!(iph && iph->protocol))
		return XT_CONTINUE;

	// get tcp hdr here
	tcph = tcp_hdr(skb);
	tcp_hdrlen = tcph->doff * 4;

	tcph->window = htons(info->win);
	
	opt = (u_int8_t *)tcph;
	for (i = sizeof(struct tcphdr); i <= tcp_hdrlen - TCPOLEN_WINDOW; i += optlen(opt, i)) {
		if (opt[i] == TCPOLEN_WINDOW && opt[i+1] == TCPOLEN_WINDOW) {
			u_int8_t oldscale;

			oldscale = opt[i+2];
	
			opt[i+2] = info->winscale;
		}
		// change scale only if window scale is enabled
	}

	offset = skb_transport_offset(skb);
	len = skb->len - offset;
	tcph->check = 0;
	tcph->check = csum_tcpudp_magic((iph->saddr), (iph->daddr), len, IPPROTO_TCP, csum_partial((char *)tcph, len, 0));
	skb->ip_summed = CHECKSUM_NONE;
	return XT_CONTINUE;
}

static int twin_tg_check(const struct xt_tgchk_param *par)
{
	return 0;
}

static struct xt_target hl_tg_reg[] __read_mostly = {
	{
		.name	   = "TCPWIN",
		.revision   = 0,
		.family	 = NFPROTO_IPV4,
		.target	 = twin_tg,
		.targetsize = sizeof(struct ipt_TWIN_info),
		.table	  = "mangle",
		.checkentry = twin_tg_check,
		.me	 = THIS_MODULE,
	},
};

static int __init hl_tg_init(void)
{
	return xt_register_targets(hl_tg_reg, ARRAY_SIZE(hl_tg_reg));
}

static void __exit hl_tg_exit(void)
{
	xt_unregister_targets(hl_tg_reg, ARRAY_SIZE(hl_tg_reg));
}

module_init(hl_tg_init);
module_exit(hl_tg_exit);
MODULE_ALIAS("ipt_TCPWIN");
