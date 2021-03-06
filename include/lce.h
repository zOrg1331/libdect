/*
 * DECT Link Control Entity (LCE)
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_LCE_H
#define _LIBDECT_LCE_H

#include <sys/socket.h>
#include <linux/dect.h>
#include <list.h>
#include <s_fmt.h>
#include <utils.h>

static inline void dect_mbuf_dump(enum dect_debug_subsys subsys,
				  const struct dect_msg_buf *mb,
				  const char *prefix)
{
	dect_hexdump(subsys, prefix, mb->data, mb->len);
}

/**
 * enum dect_transaction_role - the LCE's role in a transaction
 *
 * @DECT_TRANSACTION_INITIATOR:	Transaction was initiated by higher layer protocol entity
 * @DECT_TRANSACTION_RESPONDER:	Transaction was initiated by network
 */
enum dect_transaction_role {
	DECT_TRANSACTION_INITIATOR,
	DECT_TRANSACTION_RESPONDER,
	__DECT_TRANSACTION_MAX
};
#define DECT_TRANSACTION_MAX		(__DECT_TRANSACTION_MAX - 1)

enum dect_transaction_state {
	DECT_TRANSACTION_CLOSED,
	DECT_TRANSACTION_OPEN,
};

/* Connectionless NWK layer transaction value */
#define DECT_TV_CONNECTIONLESS		6

enum dect_release_modes {
	DECT_DDL_RELEASE_NORMAL,
	DECT_DDL_RELEASE_PARTIAL,
};

/**
 * struct dect_transaction - DECT protocol transaction
 *
 * @list:	Datalink transaction list node
 * @link:	Associated data link
 * @mb:		Message buffer for retransmissions
 * @pd:		Protocol discriminator
 * @role:	Role (initiator/responder)
 * @tv:		Transaction value
 */
struct dect_transaction {
	struct list_head		list;
	struct dect_data_link		*link;
	struct dect_msg_buf		*mb;
	enum dect_pds			pd;
	enum dect_transaction_role	role:8;
	enum dect_transaction_state	state:8;
	uint16_t			tv;
};

extern int dect_ddl_transaction_open(struct dect_handle *dh,
				     struct dect_transaction *ta,
				     struct dect_data_link *ddl,
				     enum dect_pds pd);
extern struct dect_data_link *dect_ddl_connect(struct dect_handle *dh,
					       const struct dect_ipui *ipui);
extern int dect_ddl_set_ipui(struct dect_handle *dh, struct dect_data_link *ddl,
			     const struct dect_ipui *ipui);

extern int dect_transaction_open(struct dect_handle *dh,
				 struct dect_transaction *ta,
				 const struct dect_ipui *ipui,
				 enum dect_pds pd);
extern void dect_transaction_confirm(struct dect_handle *dh,
				     struct dect_transaction *ta,
				     const struct dect_transaction *req);
extern void dect_transaction_close(struct dect_handle *dh,
				   struct dect_transaction *ta,
				   enum dect_release_modes mode);
extern void dect_transaction_get_ulei(struct sockaddr_dect_lu *addr,
				      const struct dect_transaction *ta);

extern int dect_lce_send(const struct dect_handle *dh,
			 struct dect_transaction *ta,
			 const struct dect_sfmt_msg_desc *desc,
			 const struct dect_msg_common *msg, uint8_t type);
extern int dect_lce_retransmit(const struct dect_handle *dh,
			       struct dect_transaction *ta);

extern int dect_lce_send_cl(struct dect_handle *dh, const struct dect_ipui *ipui,
			    const struct dect_sfmt_msg_desc *desc,
			    const struct dect_msg_common *msg,
			    enum dect_pds pd, uint8_t type);

extern ssize_t dect_lce_broadcast(const struct dect_handle *dh,
				  const struct dect_msg_buf *mb,
				  bool long_page, bool fast_page);

/**
 * struct dect_nwk_protocol - NWK layer protocol
 *
 * @name:	Protocol name
 * @pd:		Protocol discriminator
 * @open:	Open a new transaction initiated by the network
 * @shutdown:	Perform an active shutdown of the transaction
 * @rcv:	Receive a message related to an active transaction
 */
enum dect_cipher_states;
struct dect_nwk_protocol {
	const char		*name;
	enum dect_pds		pd;
	uint16_t		max_transactions;
	void			(*open)(struct dect_handle *dh,
				        struct dect_transaction *req,
				        struct dect_msg_buf *mb);
	void			(*shutdown)(struct dect_handle *dh,
					    struct dect_transaction *req);
	void			(*rcv)(struct dect_handle *dh,
				       struct dect_transaction *ta,
				       struct dect_msg_buf *mb);
	void			(*encrypt_ind)(struct dect_handle *dh,
					       struct dect_transaction *ta,
					       enum dect_cipher_states state);
	void			(*rebind)(struct dect_handle *dh,
					  struct dect_data_link *from,
					  struct dect_data_link *to);
};

extern void dect_lce_register_protocol(const struct dect_nwk_protocol *protocol);

/**
 * struct dect_lte - Location Table Entry
 *
 * @list:			Location table list node
 * @ipui:			International Portable User ID
 * @tpui:			Assigned Temporary Portable User ID
 * @tpui_valid:			TPUI is valid
 * @setup_capability:		PT's setup capabilities
 * @terminal_capability:	PT's terminal capabilities
 */
struct dect_lte {
	struct list_head			list;
	struct dect_ipui			ipui;
	struct dect_tpui			tpui;
	bool					tpui_valid;
	struct dect_ie_setup_capability		*setup_capability;
	struct dect_ie_terminal_capability	*terminal_capability;
};

extern void dect_lte_update(struct dect_handle *dh, const struct dect_ipui *ipui,
			    struct dect_ie_setup_capability *setup_capability,
			    struct dect_ie_terminal_capability *terminal_capability);

extern void dect_lte_update_tpui(const struct dect_handle *dh,
				 const struct dect_ipui *ipui,
				 const struct dect_tpui *tpui);

enum dect_data_link_states {
	DECT_DATA_LINK_RELEASED,
	DECT_DATA_LINK_ESTABLISHED,
	DECT_DATA_LINK_ESTABLISH_PENDING,
	DECT_DATA_LINK_RELEASE_PENDING,
	DECT_DATA_LINK_SUSPENDED,
	DECT_DATA_LINK_SUSPEND_PENDING,
	DECT_DATA_LINK_RESUME_PENDING,
	__DECT_DATA_LINK_STATE_MAX
};
#define DECT_DATA_LINK_STATE_MAX	(__DECT_DATA_LINK_STATE_MAX - 1)

enum dect_data_link_flags {
	DECT_DATA_LINK_IPUI_VALID	= 0x1,
};

/**
 * struct dect_data_link
 *
 * @list:		DECT handle link list node
 * @dlei:		Data Link Endpoint identifier
 * @ipui:		International Portable User ID
 * @dfd:		Associated socket file descriptor
 * @mcp:		MAC connections parameters
 * @state:		Data link state
 * @sdu_timer:		Establish without SDU timer (LCE.05)
 * @release_timer:	Normal link release timer (LCE.01)
 * @page_timer:		Indirect establish timer (LCE.03)
 * @page_count:		Number of page messages sent
 * @msg_queue:		Message queue used during ESTABLISH_PENDING state
 */
struct dect_data_link {
	struct list_head		list;
	struct sockaddr_dect_ssap	dlei;
	struct dect_ipui		ipui;
	struct dect_fd			*dfd;
	struct dect_mac_conn_params	mcp;
	enum dect_data_link_states	state;
	enum dect_cipher_states		cipher;
	struct dect_timer		*sdu_timer;
	struct dect_timer		*release_timer;
	struct dect_timer		*page_timer;
	uint8_t				page_count;
	uint8_t				flags;
	struct dect_msg_buf		*msg_queue;
	struct list_head		transactions;
};

#define DECT_DDL_RELEASE_TIMEOUT	5	/* LCE.01: 5 seconds */
#define DECT_DDL_LINK_MAINTAIN_TIMEOUT	5	/* LCE.02: 5 seconds */
#define DECT_DDL_PAGE_TIMEOUT		5	/* LCE.03: 5 seconds */
#define DECT_DDL_ESTABLISH_SDU_TIMEOUT	5	/* LCE.05: 5 seconds */
#define DECT_DDL_PAGE_RETRANS_MAX	3	/* N.300 */

extern int dect_ddl_set_cipher_key(const struct dect_data_link *ddl,
				   const uint8_t ck[]);
extern int dect_ddl_encrypt_req(const struct dect_data_link *ddl,
				enum dect_cipher_states status);

/* LCE message types */
enum dect_lce_msg_types {
	DECT_LCE_PAGE_RESPONSE			= 0x71,
	DECT_LCE_PAGE_REJECT			= 0x72,
};

/**
 * struct dect_lce_page_response_msg - LCE Page response S-Format message
 *
 * @common:			Message header
 * @portable_identity:		Portable Identity IE
 * @fixed_identity:		Fixed Identity IE
 * @nwk_assigned_identity:	NWK-assigned Identity IE (optional)
 * @cipher_info:		Cipher Info IE (optional)
 * @escape_to_proprietary:	Escape to proprietary IE (optional)
 */
struct dect_lce_page_response_msg {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * struct dect_lce_page_reject_msg - LCE Page reject S-Format message
 *
 * @common:			Message header
 * @portable_identity:		Portable Identity IE
 * @fixed_identity:		Fixed Identity IE
 * @reject_reason:		Reject reason IE
 * @escape_to_proprietary:	Escape to proprietary IE (optional)
 */
struct dect_lce_page_reject_msg {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

extern void dect_pp_change_pmid(struct dect_handle *dh);

extern int dect_lce_init(struct dect_handle *dh);
extern void dect_lce_exit(struct dect_handle *dh);

#endif /* _LIBDECT_LCE_H */
