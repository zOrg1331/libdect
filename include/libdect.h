#ifndef _LIBDECT_H
#define _LIBDECT_H

#include <linux/dect_netlink.h>
#include <dect/libdect.h>
#include <debug.h>
#include <list.h>
#include <lce.h>

enum dect_pp_identities {
	DECT_PP_IPUI		= 0x1,
	DECT_PP_TPUI		= 0x2,
};

/**
 * struct dect_handle - libdect handle
 *
 * @ops:	user ops
 * @nlsock:	netlink socket
 * @nlfd:	netlink file descriptor
 * @index:	cluster index
 * @mode:	cluster mode
 * @pari:	FP's PARI
 * @fpc:	FP capabilities
 * @ipui:	PP's IPUI
 * @tpui:	PP's TPUI
 * @pmid:	PP's PMID
 * @flags:	PP identity validity flags
 * @ldb:	LCE location table data base
 * @b_sap:	B-SAP socket
 * @s_sap:	S-SAP listener socket
 * @links:	list of data links
 * @mme_list:	MM endpoint list
 */
struct dect_handle {
	const struct dect_ops		*ops;

	struct nl_sock			*nlsock;
	struct dect_fd			*nlfd;

	int				index;
	enum dect_cluster_modes		mode;
	struct dect_ari			pari;
	struct dect_fp_capabilities	fpc;

	struct dect_transaction		page_transaction;
	struct dect_ipui		ipui;
	struct dect_tpui		tpui;
	uint32_t			pmid;
	uint32_t			flags;

	struct list_head		ldb;

	struct dect_fd			*b_sap;
	struct dect_fd			*s_sap;
	struct list_head		links;

	struct list_head		mme_list;

	uint8_t				priv[] __aligned(__alignof__(uint64_t));
};

#endif /* _LIBDECT_H */
