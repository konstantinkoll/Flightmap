
struct FMVersion
{
	UINT Major;
	UINT Minor;
	UINT Build;
};

struct FMLicense
{
	CHAR PurchaseID[256];
	CHAR ProductID[256];
	CHAR PurchaseDate[16];			// Either DD/MM/YYYY or DD.MM.YYYY
	CHAR Quantity[8];
	CHAR RegName[256];
	FMVersion Version;
};


// share*it
#define LICENSE_ID           "LICENSE_ID"
#define LICENSE_DATE         "LICENSE_DATE"
#define LICENSE_PRODUCT      "LICENSE_PRODUCT"
#define LICENSE_QUANTITY     "LICENSE_QUANTITY"
#define LICENSE_VERSION      "LICENSE_VERSION"
#define LICENSE_NAME         "LICENSE_NAME"
