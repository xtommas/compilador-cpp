/***********************************************************
           CODIGOS PARA UTILIZAR EN EL SCANNER
***********************************************************/

#define NADA  0LL
#define UNO   1LL

#define CVOID			 UNO			/* 1 */
#define CCHAR			(UNO << 1)		/* 2 */
#define CINT			(UNO << 2)		/* 4 */
#define CFLOAT			(UNO << 3)		/* 8 */
#define CAMPER			(UNO << 4)		/* 16 */
#define CCOR_ABR		(UNO << 5)		/* 32 */
#define CCOR_CIE		(UNO << 6)		/* 64 */
#define CCOMA			(UNO << 7)		/* 128 */
#define CPYCOMA			(UNO << 8)		/* 256 */
#define CASIGNAC		(UNO << 9)		/* 512 */
#define CLLA_ABR		(UNO << 10)		/* 1024 */
#define CLLA_CIE		(UNO << 11)		/* 2048 */
#define CWHILE			(UNO << 12)		/* 4096 */
#define CPAR_ABR		(UNO << 13)		/* 8192 */
#define CPAR_CIE		(UNO << 14)		/* ... */
#define CIF			(UNO << 15)
#define CELSE			(UNO << 16)
#define CIN			(UNO << 17)
#define COUT			(UNO << 18)
#define CSHL			(UNO << 19)		/* << */
#define CSHR			(UNO << 20)		/* >> */
#define CDISTINTO		(UNO << 21)  
#define CIGUAL			(UNO << 22)
#define CMENOR			(UNO << 23)
#define CMEIG			(UNO << 24)
#define CMAYOR			(UNO << 25)
#define CMAIG			(UNO << 26)
#define CMAS			(UNO << 27)
#define CMENOS			(UNO << 28)
#define COR			(UNO << 29)
#define CMULT			(UNO << 30)
#define CDIV			(UNO << 31)
#define CAND			(UNO << 32)
#define CIDENT			(UNO << 33)
#define CNEG			(UNO << 34)
#define CCONS_ENT		(UNO << 35)
#define CCONS_FLO		(UNO << 36)
#define CCONS_CAR		(UNO << 37)
#define CCONS_STR		(UNO << 38)
#define CRETURN			(UNO << 39)

#define CEOF			(UNO << 40)
#define SEGUIR			(UNO << 41)
