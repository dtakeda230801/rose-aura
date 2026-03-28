#pragma once

namespace RoseAuraReturnCode {
	enum class RARetCode {
		RET_ADJUSTED           =  1,
		RET_OK                 =  0,
		RET_ERR_UNKNOWN        = -1,
		RET_ERR_INVALID_ARG    = -2,
		RET_ERR_INVALID_STATE  = -3,
		RET_ERR_INVALID_PARAMS = -4,
	};
}