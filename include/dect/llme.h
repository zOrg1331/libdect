#ifndef _LIBDECT_DECT_LLME_H
#define _LIBDECT_DECT_LLME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup llme
 * @{
 */

/**
 * Fixed part capabilities
 */
struct dect_fp_capabilities {
	uint32_t	fpc;	/**< fixed part capabilities */
	uint16_t	hlc;	/**< higher layer capabilities */
	uint16_t	efpc;	/**< extended fixed part capabilities */
	uint32_t	ehlc;	/**< extended higher layer capabilities */
	uint16_t	efpc2;	/**< extended fixed part capabilities 2 */
	uint32_t	ehlc2;	/**< extended higher layer capabilities 2 */
};


struct dect_handle;

/**
 * Lower Layer Management Entity Ops.
 *
 * The LLME ops are used to register callback functions for the LLME primitives
 * invoked by libdect.
 */
struct dect_llme_ops_ {
	void	(*mac_me_info_ind)(struct dect_handle *dh,
				   const struct dect_ari *pari,
				   const struct dect_fp_capabilities *fpc);
	/**< MAC_ME_INFO-ind primtiive */
};

extern const struct dect_fp_capabilities *dect_llme_fp_capabilities(const struct dect_handle *dh);
extern int dect_llme_rfp_preload_req(struct dect_handle *dh,
				     const struct dect_fp_capabilities *fpc);

extern int dect_llme_mac_me_info_res(struct dect_handle *dh, const struct dect_ari *pari);
extern int dect_llme_scan_req(struct dect_handle *dh);

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_RAW_H */
