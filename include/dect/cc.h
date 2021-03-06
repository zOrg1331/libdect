/*
 * DECT Call-Control (CC) NWK <-> IWU interface
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_CC_H
#define _LIBDECT_DECT_CC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cc
 * @{
 */

#include <dect/ie.h>

/** MNCC_SETUP primitive parameters */
struct dect_mncc_setup_param {
	struct dect_ie_collection		common;
	struct dect_ie_basic_service		*basic_service;
	struct dect_ie_list			iwu_attributes;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_end_to_end_compatibility	*end_to_end_compatibility;
	struct dect_ie_rate_parameters		*rate_parameters;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_called_party_number	*called_party_number;
	struct dect_ie_called_party_subaddress	*called_party_subaddress;
	struct dect_ie_calling_party_number	*calling_party_number;
	struct dect_ie_calling_party_name	*calling_party_name;
	struct dect_ie_sending_complete		*sending_complete;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_codec_list		*codec_list;
};

/** MNCC_SETUP_ACK primitive parameters */
struct dect_mncc_setup_ack_param {
	struct dect_ie_collection		common;
	struct dect_ie_info_type		*info_type;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_delimiter_request	*delimiter_request;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_codec_list		*codec_list;
};

/** MNCC_RELEASE primitive parameters */
struct dect_mncc_release_param {
	struct dect_ie_collection		common;
	struct dect_ie_release_reason		*release_reason;
	struct dect_ie_identity_type		*identity_type;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_list			facility;
	struct dect_ie_display			*display;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/** MNCC_CALL_PROC primitive parameters */
struct dect_mncc_call_proc_param {
	struct dect_ie_collection		common;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_codec_list		*codec_list;
};

/** MNCC_ALERT primitive parameters */
struct dect_mncc_alert_param {
	struct dect_ie_collection		common;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_codec_list		*codec_list;
};

/** MNCC_CONNECT primitive parameters */
struct dect_mncc_connect_param {
	struct dect_ie_collection		common;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_codec_list		*codec_list;
};

/** MNCC_FACILITY primitive parameters */
struct dect_mncc_facility_param {
	struct dect_ie_collection		common;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/** MNCC_INFO primitive parameters */
struct dect_mncc_info_param {
	struct dect_ie_collection		common;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_called_party_number	*called_party_number;
	struct dect_ie_called_party_subaddress	*called_party_subaddress;
	struct dect_ie_calling_party_number	*calling_party_number;
	struct dect_ie_calling_party_name	*calling_party_name;
	struct dect_ie_sending_complete		*sending_complete;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_codec_list		*codec_list;
};

/** MNCC_MODIFY primitive parameters */
struct dect_mncc_modify_param {
	struct dect_ie_collection		common;
	struct dect_ie_service_change_info	*service_change_info;
	struct dect_ie_list			iwu_attributes;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/** MNCC_HOLD primitive parameters */
struct dect_mncc_hold_param {
	struct dect_ie_collection		common;
	struct dect_ie_display			*display;
	struct dect_ie_release_reason		*release_reason;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/** MNCC_IWU_INFO primitive parameters */
struct dect_mncc_iwu_info_param {
	struct dect_ie_collection		common;
	struct dect_ie_alphanumeric		*alphanumeric;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * Call release/rejection causes used by the @ref dect_cc_ops::mncc_release_cfm
 * "MNCC_RELEASE-cfm" and @ref dect_cc_ops::mncc_reject_ind "MNCC_REJECT-ind"
 * primitives.
 */
enum dect_causes {
	DECT_CAUSE_LOCAL_TIMER_EXPIRY,	/**< a local timer has expired */
	DECT_CAUSE_PEER_MESSAGE,	/**< a valid peer message was received */
};

struct dect_handle;
struct dect_call;
struct dect_msg_buf;

extern struct dect_call *dect_call_alloc(const struct dect_handle *dh);
extern void *dect_call_priv(struct dect_call *call);

extern const struct dect_ipui *dect_call_portable_identity(const struct dect_call *call);

/**
 * Call Control Ops.
 *
 * The CC ops are used to register callback functions for the CC primitives
 * invoked by libdect.
 */
struct dect_cc_ops {
	size_t	priv_size;
	/**< Size of the private storage area of a Call Control Endpoint */
	void	(*mncc_setup_ind)(struct dect_handle *dh, struct dect_call *call,
				  struct dect_mncc_setup_param *param);
	/**< MNCC_SETUP-ind primitive */
	void	(*mncc_setup_ack_ind)(struct dect_handle *dh, struct dect_call *call,
				      struct dect_mncc_setup_ack_param *param);
	/**< MNCC_SETUP_ACK-ind primitive */
	void	(*mncc_reject_ind)(struct dect_handle *dh, struct dect_call *call,
				   enum dect_causes cause,
				   struct dect_mncc_release_param *param);
	/**< MNCC_REJECT-ind primitive */
	void	(*mncc_call_proc_ind)(struct dect_handle *dh, struct dect_call *call,
				      struct dect_mncc_call_proc_param *param);
	/**< MNCC_CALL_PROC-ind primitve */
	void	(*mncc_alert_ind)(struct dect_handle *dh, struct dect_call *call,
				  struct dect_mncc_alert_param *param);
	/**< MNCC_ALERT-ind primitive */
	void	(*mncc_connect_ind)(struct dect_handle *dh, struct dect_call *call,
				    struct dect_mncc_connect_param *param);
	/**< MNCC_CONNECT-ind primitive */
	void	(*mncc_connect_cfm)(struct dect_handle *dh, struct dect_call *call,
				    struct dect_mncc_connect_param *param);
	/**< MNCC_CONNECT-cfm primitive */
	void	(*mncc_release_ind)(struct dect_handle *dh, struct dect_call *call,
				    struct dect_mncc_release_param *param);
	/**< MNCC_RELEASE-ind primitive */
	void	(*mncc_release_cfm)(struct dect_handle *dh, struct dect_call *call,
				    enum dect_causes cause,
				    struct dect_mncc_release_param *param);
	/**< MNCC_RELEASE-cfm primitive */
	void	(*mncc_facility_ind)(struct dect_handle *dh, struct dect_call *call,
				     struct dect_mncc_facility_param *param);
	/**< MNCC_FACILITY-ind primitive */
	void	(*mncc_info_ind)(struct dect_handle *dh, struct dect_call *call,
				 struct dect_mncc_info_param *param);
	/**< MNCC_INFO-ind primitive */
	void	(*mncc_modify_ind)(struct dect_handle *dh, struct dect_call *call,
				   struct dect_mncc_modify_param *param);
	/**< MNCC_MODIFY-ind primtiive */
	void	(*mncc_modify_cfm)(struct dect_handle *dh, struct dect_call *call,
				   bool success, struct dect_mncc_modify_param *param);
	/**< MNCC_MODIFY-cfm primitive */
	void	(*mncc_hold_ind)(struct dect_handle *dh, struct dect_call *call,
				 struct dect_mncc_hold_param *param);
	/**< MNCC_HOLD-ind primitive */
	void	(*mncc_hold_cfm)(struct dect_handle *dh, struct dect_call *call,
				 struct dect_mncc_hold_param *param);
	/**< MNCC_HOLD-cfm primitive */
	void	(*mncc_retrieve_ind)(struct dect_handle *dh, struct dect_call *call,
				     struct dect_mncc_hold_param *param);
	/**< MNCC_RETRIEVE-ind primitive */
	void	(*mncc_retrieve_cfm)(struct dect_handle *dh, struct dect_call *call,
				     struct dect_mncc_hold_param *param);
	/**< MNCC_RETRIEVE-cfm primitive */
	void	(*mncc_iwu_info_ind)(struct dect_handle *dh, struct dect_call *call,
				     struct dect_mncc_iwu_info_param *param);
	/**< MNCC_IWU_INFO-ind primitive */

	void	(*dl_u_data_ind)(struct dect_handle *dh, struct dect_call *call,
				 struct dect_msg_buf *mb);
	/**< DL_U_DATA-ind primitive */
};

extern int dect_mncc_setup_req(struct dect_handle *dh, struct dect_call *call,
			       const struct dect_ipui *ipui,
			       const struct dect_mncc_setup_param *param);
extern int dect_mncc_setup_ack_req(struct dect_handle *dh, struct dect_call *call,
				   const struct dect_mncc_setup_ack_param *param);
extern int dect_mncc_reject_req(struct dect_handle *dh, struct dect_call *call,
				const struct dect_mncc_release_param *param);
extern int dect_mncc_call_proc_req(struct dect_handle *dh, struct dect_call *call,
				   const struct dect_mncc_call_proc_param *param);
extern int dect_mncc_alert_req(struct dect_handle *dh, struct dect_call *call,
			       const struct dect_mncc_alert_param *param);
extern int dect_mncc_connect_req(struct dect_handle *dh, struct dect_call *call,
				 const struct dect_mncc_connect_param *param);
extern void dect_mncc_connect_res(struct dect_handle *dh, struct dect_call *call,
				  const struct dect_mncc_connect_param *param);
extern int dect_mncc_release_req(struct dect_handle *dh, struct dect_call *call,
				 const struct dect_mncc_release_param *param);
extern void dect_mncc_release_res(struct dect_handle *dh, struct dect_call *call,
				  const struct dect_mncc_release_param *param);
extern int dect_mncc_facility_req(struct dect_handle *dh, struct dect_call *call,
				  const struct dect_mncc_facility_param *param);
extern int dect_mncc_info_req(struct dect_handle *dh, struct dect_call *call,
			      const struct dect_mncc_info_param *param);
extern int dect_mncc_modify_req(struct dect_handle *dh, struct dect_call *call,
				const struct dect_mncc_modify_param *param);
extern void dect_mncc_modify_res(struct dect_handle *dh, struct dect_call *call,
				 bool success, const struct dect_mncc_modify_param *param);
extern int dect_mncc_hold_req(struct dect_handle *dh, struct dect_call *call,
			      const struct dect_mncc_hold_param *param);
extern void dect_mncc_hold_res(struct dect_handle *dh, struct dect_call *call,
			       const struct dect_mncc_hold_param *param);
extern int dect_mncc_retrieve_req(struct dect_handle *dh, struct dect_call *call,
				  const struct dect_mncc_hold_param *param);
extern void dect_mncc_retrieve_res(struct dect_handle *dh, struct dect_call *call,
				   const struct dect_mncc_hold_param *param);
extern int dect_mncc_iwu_info_req(struct dect_handle *dh, struct dect_call *call,
				  const struct dect_mncc_iwu_info_param *param);

extern int dect_dl_u_data_req(const struct dect_handle *dh, struct dect_call *call,
			      struct dect_msg_buf *mb);

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_CC_H */
