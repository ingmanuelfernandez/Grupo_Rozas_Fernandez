
/* Includes ------------------------------------------------------------------*/
#include "arm_math.h"

/* Exported define -----------------------------------------------------------*/
#define LP_FS16000_50_1000_LENGTH 	64
#define HP_FS16000_1000_50_LENGTH   64
#define BP_FS16000_950_1050_LENGTH  92
#define FILTER_MAX_LENGHT           92

/* Exported data -------------------------------------------------------------*/
extern const q15_t lp16000_50_1000[]; 
extern const q15_t hp16000_1000_50[];
extern const q15_t bp16000_950_1050[];

/* End of file ---------------------------------------------------------------*/
