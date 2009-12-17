/*
 * DECT Mobility Management (MM)
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/dect.h>

#include <libdect.h>
#include <utils.h>
#include <s_fmt.h>
#include <lce.h>
#include <mm.h>
#include <dect/auth.h>

static DECT_SFMT_MSG_DESC(mm_access_rights_accept,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_MANDATORY, IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ZAP_FIELD,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SERVICE_CLASS,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_request,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_reject,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_terminate_accept,
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_terminate_reject,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_terminate_request,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_reject,
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	//DECT_SFMT_IE(S_VL_IE_AUTH_REJECT_PARAMETER,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_reply,
	DECT_SFMT_IE(S_VL_IE_RES,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ZAP_FIELD,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SERVICE_CLASS,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_KEY,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_request,
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RAND,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RES,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_suggest,
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_CALL_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_request,
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_reject,
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_detach,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_identity_reply,
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_identity_request,
	DECT_SFMT_IE(S_VL_IE_IDENTITY_TYPE,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_key_allocate,
	DECT_SFMT_IE(S_VL_IE_ALLOCATION_TYPE,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_RAND,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_accept,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_SE_IE_USE_TPUI,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_reject,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_request,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_info_accept,
	DECT_SFMT_IE(S_VL_IE_INFO_TYPE,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_info_reject,
	DECT_SFMT_IE(S_VL_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_info_request,
	DECT_SFMT_IE(S_VL_IE_INFO_TYPE,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CALL_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_info_suggest,
	DECT_SFMT_IE(S_VL_IE_INFO_TYPE,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_KEY,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign_ack,
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign_rej,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_notify_msg,
	DECT_SFMT_IE(S_DO_IE_TIMER_RESTART,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

#define mm_debug(mme, fmt, args...) \
	dect_debug("MM (link %d): " fmt "\n", (mme)->link->dfd->fd, ## args)

void *dect_mm_priv(struct dect_mm_endpoint *mme)
{
	return mme->priv;
}

static struct dect_mm_endpoint *
dect_mm_endpoint_get_by_link(const struct dect_handle *dh,
			     const struct dect_data_link *link)
{
	struct dect_mm_endpoint *mme;

	list_for_each_entry(mme, &dh->mme_list, list) {
		if (mme->link == link)
			return mme;
	}
	return NULL;
}

struct dect_mm_endpoint *dect_mm_endpoint_alloc(struct dect_handle *dh)
{
	struct dect_mm_endpoint *mme;

	mme = dect_zalloc(dh, sizeof(*mme) + dh->ops->mm_ops->priv_size);
	if (mme == NULL)
		goto err1;

	mme->procedure[DECT_TRANSACTION_INITIATOR].timer = dect_alloc_timer(dh);
	if (mme->procedure[DECT_TRANSACTION_INITIATOR].timer == NULL)
		goto err2;

	mme->procedure[DECT_TRANSACTION_RESPONDER].timer = dect_alloc_timer(dh);
	if (mme->procedure[DECT_TRANSACTION_RESPONDER].timer == NULL)
		goto err3;

	list_add_tail(&mme->list, &dh->mme_list);
	return mme;

err3:
	dect_free(dh, mme->procedure[DECT_TRANSACTION_INITIATOR].timer);
err2:
	dect_free(dh, mme);
err1:
	return NULL;
}

static void dect_mm_endpoint_destroy(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme)
{
	dect_free(dh, mme->procedure[DECT_TRANSACTION_RESPONDER].timer);
	dect_free(dh, mme->procedure[DECT_TRANSACTION_INITIATOR].timer);
	dect_free(dh, mme);
}

static struct dect_mm_endpoint *dect_mm_endpoint(struct dect_transaction *ta)
{
	return container_of(ta, struct dect_mm_endpoint, procedure[ta->role].transaction);
}

static int dect_mm_send_msg(struct dect_handle *dh,
			    const struct dect_mm_endpoint *mme,
			    enum dect_transaction_role role,
			    const struct dect_sfmt_msg_desc *desc,
			    const struct dect_msg_common *msg,
			    enum dect_mm_msg_types type)
{
	return dect_lce_send(dh, &mme->procedure[role].transaction, desc, msg, type);
}

/**
 * dect_mm_key_allocate_req - MM_KEY_ALLOCATE-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	key allocate request parameters
 */
int dect_mm_key_allocate_req(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme,
			     const struct dect_mm_key_allocate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_key_allocate_msg msg;
	int err;

	mm_debug(mme, "KEY_ALLOCATE-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
					DECT_PD_MM);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.allocation_type	= param->allocation_type;
	msg.rand		= param->rand;
	msg.rs			= param->rs;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
			       &mm_key_allocate_msg_desc,
			       &msg.common, DECT_MM_KEY_ALLOCATE);
	if (err < 0)
		goto err2;

	mp->type = DECT_MMP_KEY_ALLOCATION;
	return 0;

err2:
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return err;
}

static void dect_mm_rcv_key_allocate(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_key_allocate_msg msg;
	struct dect_mm_key_allocate_param *param;

	mm_debug(mme, "KEY-ALLOCATE");
	if (mp->type != 0)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_key_allocate_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->allocation_type	= dect_ie_hold(msg.allocation_type);
	param->rand		= dect_ie_hold(msg.rand);
	param->rs		= dect_ie_hold(msg.rs);

	dh->ops->mm_ops->mm_key_allocate_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_key_allocate_msg_desc, &msg.common);
}

/**
 * dect_mm_authenticate_req - MM_AUTHENTICATE-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	authenticate request parameters
 */
int dect_mm_authenticate_req(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme,
			     const struct dect_mm_authenticate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_authentication_request_msg msg;
	int err;

	mm_debug(mme, "AUTHENTICATE-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
					DECT_PD_MM);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.auth_type			= param->auth_type;
	msg.rand			= param->rand;
	msg.res				= param->res;
	msg.rs				= param->rs;
	msg.cipher_info			= param->cipher_info;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
			       &mm_authentication_request_msg_desc,
			       &msg.common, DECT_MM_AUTHENTICATION_REQUEST);
	if (err < 0)
		goto err2;

	mp->type = DECT_MMP_AUTHENTICATE;
	return 0;

err2:
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return err;
}

/**
 * dect_mm_authenticate_req - MM_AUTHENTICATE-res primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	authenticate response parameters
 */
int dect_mm_authenticate_res(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme,
			     const struct dect_mm_authenticate_param *param)
{
	return 0;
}

static void dect_mm_rcv_authentication_request(struct dect_handle *dh,
					       struct dect_mm_endpoint *mme,
					       struct dect_msg_buf *mb)
{
	struct dect_mm_authentication_request_msg msg;

	mm_debug(mme, "AUTHENTICATION-REQUEST");
	if (dect_parse_sfmt_msg(dh, &mm_authentication_request_msg_desc,
				&msg.common, mb) < 0)
		return;
}

static void dect_mm_rcv_authentication_reply(struct dect_handle *dh,
					     struct dect_mm_endpoint *mme,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_authentication_reply_msg msg;
	struct dect_mm_authenticate_param *param;

	mm_debug(mme, "AUTHENTICATION-REPLY");
	if (mp->type != DECT_MMP_AUTHENTICATE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_authentication_reply_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->res			= dect_ie_hold(msg.res);
	param->rs			= dect_ie_hold(msg.rs);
	param->zap_field		= dect_ie_hold(msg.zap_field);
	param->service_class		= dect_ie_hold(msg.service_class);
	param->key			= dect_ie_hold(msg.key);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_authenticate_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_authentication_reply_msg_desc, &msg.common);
}

static void dect_mm_rcv_authentication_reject(struct dect_handle *dh,
					      struct dect_mm_endpoint *mme,
					      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_authentication_reject_msg msg;
	struct dect_mm_authenticate_param *param;

	mm_debug(mme, "AUTHENTICATION-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_authentication_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	//param->auth_type	= *dect_ie_list_hold(&msg.auth_type);
	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_authenticate_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_authentication_reject_msg_desc, &msg.common);
}

/**
 * dect_mm_cipher_req - MM_CIPHER-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	cipher request parameters
 * @ck:		cipher key
 */
int dect_mm_cipher_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       const struct dect_mm_cipher_param *param,
		       const uint8_t ck[DECT_CIPHER_KEY_LEN])
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_cipher_request_msg msg;
	int err;

	mm_debug(mme, "CIPHER-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
					DECT_PD_MM);
	if (err < 0)
		goto err1;

	err = dect_ddl_set_cipher_key(mme->link, ck);
	if (err < 0)
		goto err2;

	memset(&msg, 0, sizeof(msg));
	msg.cipher_info			= param->cipher_info;
	msg.call_identity		= param->call_identity;
	msg.connection_identity		= param->connection_identity;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	/* cipher_request and cipher_suggest messages have identical layout */
	if (dh->mode == DECT_MODE_FP)
		err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
				       &mm_cipher_request_msg_desc,
				       &msg.common, DECT_MM_CIPHER_REQUEST);
	else
		err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
				       &mm_cipher_suggest_msg_desc,
				       &msg.common, DECT_MM_CIPHER_SUGGEST);

	if (err < 0)
		goto err2;

	mp->type = DECT_MMP_CIPHER;
	return 0;

err2:
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return err;
}

/**
 * dect_mm_cipher_res - MM_CIPHER-res primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @accept:	accept/reject ciphering
 * @param:	cipher respond parameters
 * @ck:		cipher key
 */
int dect_mm_cipher_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       bool accept, const struct dect_mm_cipher_param *param,
		       const uint8_t ck[DECT_CIPHER_KEY_LEN])
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_cipher_reject_msg rmsg;
	int err;

	if (mp->type != DECT_MMP_CIPHER)
		return -1;

	if (accept) {
		err = dect_ddl_set_cipher_key(mme->link, ck);
		if (err < 0)
			goto err1;

		err = dect_ddl_encrypt_req(mme->link, DECT_CIPHER_ENABLED);
		if (err < 0)
			goto err1;
	} else {
		memset(&rmsg, 0, sizeof(rmsg));
		//rmsg.cipher_info		= param->cipher_info;
		rmsg.reject_reason		= param->reject_reason;
		rmsg.escape_to_proprietary	= param->escape_to_proprietary;
	}

	return 0;

err1:
	return err;
}

static void dect_mm_rcv_cipher_request(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_cipher_request_msg msg;
	struct dect_mm_cipher_param *param;

	mm_debug(mme, "CIPHER-REQUEST");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_cipher_request_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->connection_identity	= dect_ie_hold(msg.connection_identity);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dh->ops->mm_ops->mm_cipher_ind(dh, mme, param);
err1:
	dect_msg_free(dh, &mm_cipher_request_msg_desc, &msg.common);
}

static void dect_mm_rcv_cipher_suggest(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_cipher_suggest_msg msg;
	struct dect_mm_cipher_param *param;

	mm_debug(mme, "CIPHER-SUGGEST");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_cipher_suggest_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->connection_identity	= dect_ie_hold(msg.connection_identity);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dh->ops->mm_ops->mm_cipher_ind(dh, mme, param);
err1:
	dect_msg_free(dh, &mm_cipher_suggest_msg_desc, &msg.common);
}

static void dect_mm_rcv_cipher_reject(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_cipher_reject_msg msg;
	struct dect_mm_cipher_param *param;

	mm_debug(mme, "CIPHER-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_cipher_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	//param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_cipher_cfm(dh, mme, false, param);
err1:
	dect_msg_free(dh, &mm_cipher_reject_msg_desc, &msg.common);
}

static void dect_mm_cipher_cfm(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];

	mm_debug(mme, "CIPHER-cfm");
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_cipher_cfm(dh, mme, true, NULL);
}

static void dect_mm_encrypt_ind(struct dect_handle *dh, struct dect_transaction *ta,
				enum dect_cipher_states state)
{
	struct dect_mm_endpoint *mme = dect_mm_endpoint(ta);
	struct dect_mm_procedure *mp = &mme->procedure[ta->role];

	if (mp->type == DECT_MMP_CIPHER)
		dect_mm_cipher_cfm(dh, mme);
}

/**
 * dect_mm_access_rights_req - MM_ACCESS_RIGHTS-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	access rights request parameters
 */
int dect_mm_access_rights_req(struct dect_handle *dh,
			      struct dect_mm_endpoint *mme,
			      const struct dect_mm_access_rights_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_access_rights_request_msg msg;
	int err;

	mm_debug(mme, "ACCESS_RIGHTS-req");
	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
				        DECT_PD_MM);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.auth_type			= param->auth_type;
	msg.cipher_info			= param->cipher_info;
	msg.setup_capability		= NULL;
	msg.terminal_capability		= param->terminal_capability;
	msg.model_identifier		= param->model_identifier;
	msg.codec_list			= NULL;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
			       &mm_access_rights_request_msg_desc,
			       &msg.common, DECT_MM_ACCESS_RIGHTS_REQUEST);
	if (err < 0)
		goto err2;

	mp->type = DECT_MMP_ACCESS_RIGHTS;
	return 0;

err2:
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return -1;
}

/**
 * dect_mm_access_rights_res - MM_ACCESS_RIGHTS-res primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	access rights response parameters
 */
int dect_mm_access_rights_res(struct dect_handle *dh,
			      struct dect_mm_endpoint *mme, bool accept,
			      const struct dect_mm_access_rights_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_access_rights_accept_msg msg;
	struct dect_ie_fixed_identity fixed_identity;
	int err;

	mm_debug(mme, "ACCESS_RIGHTS-res");
	if (mp->type != DECT_MMP_ACCESS_RIGHTS)
		return -1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.fixed_identity		= param->fixed_identity;
	msg.auth_type			= param->auth_type;
	msg.location_area		= param->location_area;
	msg.cipher_info			= param->cipher_info;
	msg.setup_capability		= NULL;
	msg.model_identifier		= param->model_identifier;
	//msg.iwu_to_iwu		= param->iwu_to_iwu;
	//msg.codec_list		= param->codec_list;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	if (param->fixed_identity.list == NULL) {
		fixed_identity.type = DECT_FIXED_ID_TYPE_PARK;
		fixed_identity.ari = dh->pari;
		fixed_identity.rpn = 0;
		dect_ie_list_add(&fixed_identity, &msg.fixed_identity);
	}

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_RESPONDER,
			       &mm_access_rights_accept_msg_desc,
			       &msg.common, DECT_MM_ACCESS_RIGHTS_ACCEPT);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	dect_mm_endpoint_destroy(dh, mme);
	return err;
}

static void dect_mm_rcv_access_rights_request(struct dect_handle *dh,
					      struct dect_mm_endpoint *mme,
					      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_access_rights_request_msg msg;
	struct dect_mm_access_rights_param *param;

	mm_debug(mme, "ACCESS-RIGHTS-REQUEST");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_access_rights_request_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->auth_type		= dect_ie_hold(msg.auth_type);
	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->terminal_capability	= dect_ie_hold(msg.terminal_capability);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mp->type = DECT_MMP_ACCESS_RIGHTS;

	dh->ops->mm_ops->mm_access_rights_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_access_rights_request_msg_desc, &msg.common);
}

static void dect_mm_rcv_access_rights_accept(struct dect_handle *dh,
					     struct dect_mm_endpoint *mme,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_access_rights_accept_msg msg;
	struct dect_mm_access_rights_param *param;

	mm_debug(mme, "ACCESS-RIGHTS-ACCEPT");
	if (dect_parse_sfmt_msg(dh, &mm_access_rights_accept_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->fixed_identity		= *dect_ie_list_hold(&msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->auth_type		= dect_ie_hold(msg.auth_type);
	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->zap_field		= dect_ie_hold(msg.zap_field);
	param->service_class		= dect_ie_hold(msg.service_class);
	//param->setup_capability	= dect_ie_hold(msg.setup_capability);
	param->model_identifier		= dect_ie_hold(msg.model_identifier);
	//param->codec_list		= dect_ie_hold(msg.codec_list);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_access_rights_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_access_rights_accept_msg_desc, &msg.common);
}

static void dect_mm_rcv_access_rights_reject(struct dect_handle *dh,
					     struct dect_mm_endpoint *mme,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_access_rights_reject_msg msg;
	struct dect_mm_access_rights_param *param;

	mm_debug(mme, "ACCESS-RIGHTS-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_access_rights_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->duration			= dect_ie_hold(msg.duration);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_access_rights_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_access_rights_reject_msg_desc, &msg.common);
}

static int dect_mm_send_locate_accept(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_locate_accept_msg msg = {
		.portable_identity	= param->portable_identity,
		.location_area		= param->location_area,
		.nwk_assigned_identity	= param->nwk_assigned_identity,
		.duration		= param->duration,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
		.model_identifier	= param->model_identifier,
	};
	int err;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_RESPONDER,
			       &mm_locate_accept_msg_desc,
			       &msg.common, DECT_MM_LOCATE_ACCEPT);
	if (err < 0)
		return err;

	/*
	 * TPUI or NWK identity assignment begins an identity assignment
	 * procedure.
	 */
	if (param->portable_identity->type == DECT_PORTABLE_ID_TYPE_TPUI ||
	    param->nwk_assigned_identity)
		mp->type = DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT;
	else {
		dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
		mp->type = DECT_MMP_NONE;
	}

	return err;
}

static int dect_mm_send_locate_reject(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_locate_reject_msg msg = {
		.reject_reason		= param->reject_reason,
		.duration		= param->duration,
		.segmented_info		= {},
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};
	int err;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_RESPONDER,
			       &mm_locate_reject_msg_desc,
			       &msg.common, DECT_MM_LOCATE_REJECT);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;
	return err;
}

int dect_mm_locate_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       const struct dect_mm_locate_param *param)
{
	if (param->reject_reason == NULL)
		return dect_mm_send_locate_accept(dh, mme, param);
	else
		return dect_mm_send_locate_reject(dh, mme, param);
}

static void dect_mm_rcv_locate_request(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_locate_request_msg msg;
	struct dect_mm_locate_param *param;

	mm_debug(mme, "LOCATE-REQUEST");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_locate_request_msg_desc,
				&msg.common, mb) < 0)
		goto err1;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->portable_identity	= dect_ie_hold(msg.portable_identity),
	param->fixed_identity		= dect_ie_hold(msg.fixed_identity),
	param->location_area		= dect_ie_hold(msg.location_area),
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity),
	param->cipher_info		= dect_ie_hold(msg.cipher_info),
	param->setup_capability		= dect_ie_hold(msg.setup_capability),
	param->terminal_capability	= dect_ie_hold(msg.terminal_capability),
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu),
	param->model_identifier		= dect_ie_hold(msg.model_identifier),

	mp->type = DECT_MMP_LOCATION_REGISTRATION;;
	dh->ops->mm_ops->mm_locate_ind(dh, mme, param);

	dect_ie_collection_put(dh, param);
err2:
	dect_msg_free(dh, &mm_locate_request_msg_desc, &msg.common);
err1:
	return;
}

static void dect_mm_rcv_locate_accept(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_locate_accept_msg msg;

	mm_debug(mme, "LOCATE-ACCEPT");
	if (dect_parse_sfmt_msg(dh, &mm_locate_accept_msg_desc,
				&msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &mm_locate_accept_msg_desc, &msg.common);
}

static void dect_mm_rcv_locate_reject(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_locate_reject_msg msg;

	mm_debug(mme, "LOCATE-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_locate_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &mm_locate_reject_msg_desc, &msg.common);
}

/**
 * dect_mm_detach_req - MM_DETACH-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	detach parameters
 */
int dect_mm_detach_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       struct dect_mm_detach_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_detach_msg msg;
	int err;

	mm_debug(mme, "DETACH-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
				        DECT_PD_MM);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.nwk_assigned_identity	= param->nwk_assigned_identity;
	msg.iwu_to_iwu			= param->iwu_to_iwu;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
			       &mm_detach_msg_desc,
			       &msg.common, DECT_MM_DETACH);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return err;
}

static void dect_mm_rcv_detach(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme,
			       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_detach_param *param;
	struct dect_mm_detach_msg msg;

	mm_debug(mme, "DETACH");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_detach_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	dh->ops->mm_ops->mm_detach_ind(dh, mme, param);

	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_detach_msg_desc, &msg.common);
}

static void dect_mm_rcv_identity_request(struct dect_handle *dh,
					 struct dect_mm_endpoint *mme,
					 struct dect_msg_buf *mb)
{
	struct dect_mm_identity_request_msg msg;

	mm_debug(mme, "IDENTITY-REQUEST");
	if (dect_parse_sfmt_msg(dh, &mm_identity_request_msg_desc,
				&msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &mm_identity_request_msg_desc, &msg.common);
}

static void dect_mm_rcv_identity_reply(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_identity_reply_msg msg;

	mm_debug(mme, "IDENTITY-REPLY");
	if (dect_parse_sfmt_msg(dh, &mm_identity_reply_msg_desc,
				&msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &mm_identity_reply_msg_desc, &msg.common);
}

static void dect_mm_rcv_temporary_identity_assign_ack(struct dect_handle *dh,
						      struct dect_mm_endpoint *mme,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_temporary_identity_assign_ack_msg msg;
	struct dect_mm_identity_assign_param *param;

	mm_debug(mme, "TEMPORARY-IDENTITY-ASSIGN-ACK");
	if (mp->type != DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_ack_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mme, true, param);
err1:
	dect_msg_free(dh, &mm_temporary_identity_assign_ack_msg_desc, &msg.common);
}

static void dect_mm_rcv_temporary_identity_assign_rej(struct dect_handle *dh,
						      struct dect_mm_endpoint *mme,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_temporary_identity_assign_rej_msg msg;
	struct dect_mm_identity_assign_param *param;

	mm_debug(mme, "TEMPORARY-IDENTITY-ASSIGN-REJ");
	if (mp->type != DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_rej_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->reject_reason 		= dect_ie_hold(msg.reject_reason);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_temporary_identity_assign_rej_msg_desc, &msg.common);
}

/**
 * dect_mm_info_req - MM_INFO-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	info parameters
 */
int dect_mm_info_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		     struct dect_mm_info_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_info_request_msg rmsg;
	struct dect_mm_info_suggest_msg smsg;
	int err;

	mm_debug(mme, "INFO-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
					DECT_PD_MM);
	if (err < 0)
		goto err1;

	if (dh->mode == DECT_MODE_FP) {
		memset(&smsg, 0, sizeof(smsg));
		smsg.info_type			= param->info_type;
		smsg.call_identity		= param->call_identity;
		//smsg.fixed_identity		= param->fixed_identity;
		smsg.location_area		= param->location_area;
		smsg.nwk_assigned_identity	= param->nwk_assigned_identity;
		smsg.network_parameter		= param->network_parameter;
		smsg.iwu_to_iwu			= param->iwu_to_iwu;
		smsg.escape_to_proprietary	= param->escape_to_proprietary;

		err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
				       &mm_info_suggest_msg_desc,
				       &smsg.common, DECT_MM_INFO_SUGGEST);
		if (err < 0)
			goto err2;

		dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	} else {
		memset(&rmsg, 0, sizeof(rmsg));
		rmsg.info_type			= param->info_type;
		rmsg.call_identity		= param->call_identity;
		rmsg.portable_identity		= param->portable_identity;
		rmsg.fixed_identity		= param->fixed_identity;
		rmsg.location_area		= param->location_area;
		rmsg.nwk_assigned_identity	= param->nwk_assigned_identity;
		rmsg.network_parameter		= param->network_parameter;
		rmsg.iwu_to_iwu			= param->iwu_to_iwu;
		rmsg.escape_to_proprietary	= param->escape_to_proprietary;

		err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
				       &mm_info_request_msg_desc,
				       &rmsg.common, DECT_MM_INFO_REQUEST);
		if (err < 0)
			goto err2;

		mp->type = DECT_MMP_PARAMETER_RETRIEVAL;
	}

	return 0;
err2:
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return err;
}

/**
 * dect_mm_info_res - MM_INFO-res primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	info parameters
 */
int dect_mm_info_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		     bool accept, struct dect_mm_info_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_info_accept_msg amsg;
	struct dect_mm_info_reject_msg rmsg;
	int err;

	if (mp->type != DECT_MMP_PARAMETER_RETRIEVAL)
		return -1;

	if (accept) {
		memset(&amsg, 0, sizeof(amsg));
		amsg.info_type			= param->info_type;
		amsg.call_identity		= param->call_identity;
		amsg.fixed_identity		= param->fixed_identity;
		amsg.location_area		= param->location_area;
		amsg.nwk_assigned_identity	= param->nwk_assigned_identity;
		amsg.network_parameter		= param->network_parameter;
		amsg.iwu_to_iwu			= param->iwu_to_iwu;
		amsg.escape_to_proprietary	= param->escape_to_proprietary;

		err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_RESPONDER,
				       &mm_info_accept_msg_desc,
				       &amsg.common, DECT_MM_INFO_ACCEPT);
	} else {
		memset(&rmsg, 0, sizeof(rmsg));
		rmsg.call_identity		= param->call_identity;
		rmsg.reject_reason		= param->reject_reason;
		rmsg.iwu_to_iwu			= param->iwu_to_iwu;
		rmsg.escape_to_proprietary	= param->escape_to_proprietary;

		err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_RESPONDER,
				       &mm_info_reject_msg_desc,
				       &rmsg.common, DECT_MM_INFO_REJECT);
	}

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	return 0;
}

static void dect_mm_rcv_info_request(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_info_request_msg msg;
	struct dect_mm_info_param *param;

	mm_debug(mme, "INFO-REQUEST");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_info_request_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->info_type		= dect_ie_hold(msg.info_type);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->fixed_identity		= dect_ie_hold(msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->network_parameter	= dect_ie_hold(msg.network_parameter);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mp->type = DECT_MMP_PARAMETER_RETRIEVAL;

	dh->ops->mm_ops->mm_info_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_info_request_msg_desc, &msg.common);
}

static void dect_mm_rcv_info_accept(struct dect_handle *dh,
				    struct dect_mm_endpoint *mme,
				    struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_info_accept_msg msg;
	struct dect_mm_info_param *param;

	mm_debug(mme, "INFO-ACCEPT");
	if (mp->type != DECT_MMP_PARAMETER_RETRIEVAL)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_info_accept_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->info_type		= dect_ie_hold(msg.info_type);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->fixed_identity		= dect_ie_hold(msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->network_parameter	= dect_ie_hold(msg.network_parameter);
	param->duration			= dect_ie_hold(msg.duration);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_info_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_info_accept_msg_desc, &msg.common);
}

static void dect_mm_rcv_info_reject(struct dect_handle *dh,
				    struct dect_mm_endpoint *mme,
				    struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_info_reject_msg msg;
	struct dect_mm_info_param *param;

	mm_debug(mme, "INFO-REJECT");
	if (mp->type != DECT_MMP_PARAMETER_RETRIEVAL)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_info_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_info_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_info_reject_msg_desc, &msg.common);

}

static void dect_mm_rcv_info_suggest(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     struct dect_msg_buf *mb)
{
	struct dect_mm_info_suggest_msg msg;
	struct dect_mm_info_param *param;

	mm_debug(mme, "INFO-SUGGEST");
	if (dect_parse_sfmt_msg(dh, &mm_info_suggest_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->info_type		= dect_ie_hold(msg.info_type);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	//param->fixed_identity		= dect_ie_hold(msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->network_parameter	= dect_ie_hold(msg.network_parameter);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);

	dh->ops->mm_ops->mm_info_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_info_suggest_msg_desc, &msg.common);
}

static void dect_mm_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			struct dect_msg_buf *mb)
{
	struct dect_mm_endpoint *mme = dect_mm_endpoint(ta);

	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REPLY:
		return dect_mm_rcv_authentication_reply(dh, mme, mb);
	case DECT_MM_AUTHENTICATION_REJECT:
		return dect_mm_rcv_authentication_reject(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_ACCEPT:
		return dect_mm_rcv_access_rights_accept(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_REJECT:
		return dect_mm_rcv_access_rights_reject(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REQUEST:
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_ACCEPT:
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REJECT:
		break;
	case DECT_MM_CIPHER_REQUEST:
		return dect_mm_rcv_cipher_request(dh, mme, mb);
	case DECT_MM_CIPHER_SUGGEST:
		return dect_mm_rcv_cipher_suggest(dh, mme, mb);
	case DECT_MM_CIPHER_REJECT:
		return dect_mm_rcv_cipher_reject(dh, mme, mb);
	case DECT_MM_INFO_ACCEPT:
		return dect_mm_rcv_info_accept(dh, mme, mb);
	case DECT_MM_INFO_REJECT:
		return dect_mm_rcv_info_reject(dh, mme, mb);
	case DECT_MM_LOCATE_ACCEPT:
		return dect_mm_rcv_locate_accept(dh, mme, mb);
	case DECT_MM_LOCATE_REJECT:
		return dect_mm_rcv_locate_reject(dh, mme, mb);
	case DECT_MM_IDENTITY_REPLY:
		return dect_mm_rcv_identity_reply(dh, mme, mb);
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN:
		break;
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_ACK:
		return dect_mm_rcv_temporary_identity_assign_ack(dh, mme, mb);
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_REJ:
		return dect_mm_rcv_temporary_identity_assign_rej(dh, mme, mb);
	}

	mm_debug(mme, "receive unknown msg type %x", mb->type);
}

static void dect_mm_open(struct dect_handle *dh,
			 const struct dect_transaction *req,
			 struct dect_msg_buf *mb)
{
	struct dect_mm_endpoint *mme;
	struct dect_transaction *ta;

	dect_debug("MM: unknown transaction msg type: %x\n", mb->type);

	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
	case DECT_MM_CIPHER_REQUEST:
	case DECT_MM_CIPHER_SUGGEST:
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
	case DECT_MM_LOCATE_REQUEST:
	case DECT_MM_KEY_ALLOCATE:
	case DECT_MM_INFO_REQUEST:
	case DECT_MM_INFO_SUGGEST:
	case DECT_MM_IDENTITY_REQUEST:
	case DECT_MM_DETACH:
		break;
	default:
		return;
	}

	mme = dect_mm_endpoint_get_by_link(dh, req->link);
	if (mme == NULL) {
		mme = dect_mm_endpoint_alloc(dh);
		if (mme == NULL)
			return;
		mme->link = req->link;
	}

	ta = &mme->procedure[DECT_TRANSACTION_RESPONDER].transaction;
	dect_confirm_transaction(dh, ta, req);

	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
		return dect_mm_rcv_authentication_request(dh, mme, mb);
	case DECT_MM_CIPHER_REQUEST:
		return dect_mm_rcv_cipher_request(dh, mme, mb);
	case DECT_MM_CIPHER_SUGGEST:
		return dect_mm_rcv_cipher_suggest(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
		return dect_mm_rcv_access_rights_request(dh, mme, mb);
	case DECT_MM_LOCATE_REQUEST:
		return dect_mm_rcv_locate_request(dh, mme, mb);
	case DECT_MM_KEY_ALLOCATE:
		return dect_mm_rcv_key_allocate(dh, mme, mb);
	case DECT_MM_INFO_REQUEST:
		return dect_mm_rcv_info_request(dh, mme, mb);
	case DECT_MM_INFO_SUGGEST:
		return dect_mm_rcv_info_suggest(dh, mme, mb);
	case DECT_MM_IDENTITY_REQUEST:
		return dect_mm_rcv_identity_request(dh, mme, mb);
	case DECT_MM_DETACH:
		return dect_mm_rcv_detach(dh, mme, mb);
	default:
		BUG();
	}
}

static void dect_mm_shutdown(struct dect_handle *dh,
			     struct dect_transaction *ta)
{
	struct dect_mm_endpoint *mme = dect_mm_endpoint(ta);

	mm_debug(mme, "shutdown");
	dect_close_transaction(dh, ta, DECT_DDL_RELEASE_NORMAL);
}

static const struct dect_nwk_protocol mm_protocol = {
	.name			= "Mobility Management",
	.pd			= DECT_PD_MM,
	.max_transactions	= 1,
	.open			= dect_mm_open,
	.shutdown		= dect_mm_shutdown,
	.rcv			= dect_mm_rcv,
	.encrypt_ind		= dect_mm_encrypt_ind,
};

static void __init dect_mm_init(void)
{
	dect_lce_register_protocol(&mm_protocol);
}
