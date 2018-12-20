// CapiFunc.h: interface for the CCapiFunc class.
//
//////////////////////////////////////////////////////////////////////

//******************************************************************************
//Типы функций из библиотеки CAPI32.DLL
typedef DWORD (APIENTRY CAPIREGISTER) ( DWORD MessageBufferSize,
										DWORD maxLogicalConnection,
										DWORD maxBDataBlocks,
										DWORD maxBDataLen,
										DWORD * pApplID );

typedef DWORD (APIENTRY CAPIRELEASE) (DWORD ApplID);

typedef DWORD (APIENTRY CAPIPUTMESSAGE) ( DWORD ApplID,PVOID pCAPIMessage);

typedef DWORD (APIENTRY CAPIGETMESSAGE) ( DWORD ApplID,PVOID * ppCAPIMessage);

typedef DWORD (APIENTRY CAPIWAITFORSIGNAL) ( DWORD ApplID);

typedef VOID (APIENTRY CAPIGETMANUFACTURER) (char * SzBuffer);

typedef DWORD (APIENTRY CAPIGETVERSION) ( DWORD * pCAPIMajor,
										  DWORD * pCAPIMinor,
										  DWORD * pManufacturerMajor,
										  DWORD * pManufacturerMinor );

typedef DWORD (APIENTRY CAPIGETSERIALNUMBER) (char * SzBuffer);


typedef DWORD (APIENTRY CAPIGETPROFILE) ( PVOID SzBuffer,DWORD CtrlNr );

typedef DWORD (APIENTRY CAPIINSTALLED) (VOID);

//******************************************************************************
//messages
#define ALERT_REQUEST					0x8001
#define ALERT_CONFIRM					0x8101
#define CONNECT_REQUEST					0x8002
#define CONNECT_CONFIRM					0x8102
#define CONNECT_INDICAT					0x8202
#define CONNECT_RESPONS					0x8302
#define CONNECT_ACTIVE_INDICAT			0x8203
#define CONNECT_ACTIVE_RESPONS			0x8303
#define	CONNECT_B3_ACTIVE_INDICAT		0x8283
#define CONNECT_B3_ACTIVE_RESPONS		0x8383
#define CONNECT_B3_REQUEST				0x8082
#define CONNECT_B3_CONFIRM				0x8182
#define CONNECT_B3_INDICAT				0x8282
#define CONNECT_B3_RESPONS				0x8382
#define CONNECT_B3_T90_ACTIVE_INDICAT	0x8288
#define CONNECT_B3_T90_ACTIVE_RESPONS	0x8388
#define DATA_B3_REQUEST					0x8086
#define DATA_B3_CONFIRM					0x8186
#define DATA_B3_INDICAT					0x8286
#define DATA_B3_RESPONS					0x8386
#define DISCONNECT_B3_REQUEST			0x8084
#define DISCONNECT_B3_CONFIRM			0x8184
#define DISCONNECT_B3_INDICAT			0x8284
#define DISCONNECT_B3_RESPONS			0x8384
#define DISCONNECT_REQUEST				0x8004
#define DISCONNECT_CONFIRM				0x8104
#define DISCONNECT_INDICAT				0x8204
#define DISCONNECT_RESPONS				0x8304
#define FACILITY_REQUEST				0x8080
#define FACILITY_CONFIRM				0x8180
#define FACILITY_INDICAT				0x8280
#define FACILITY_RESPONS				0x8380
#define INFO_REQUEST					0x8008
#define INFO_CONFIRM					0x8108
#define INFO_INDICAT					0x8208
#define INFO_RESPONS					0x8308
#define LISTEN_REQUEST					0x8005
#define LISTEN_CONFIRM					0x8105
#define MANUFACTURER_REQUEST			0x80FF
#define MANUFACTURER_CONFIRM			0x81FF
#define MANUFACTURER_INDICAT			0x82FF
#define MANUFACTURER_RESPONS			0x83FF
#define RESET_B3_REQUEST				0x8087
#define RESET_B3_CONFIRM				0x8187
#define RESET_B3_INDICAT				0x8287
#define RESET_B3_RESPONS				0x8387
#define SELECT_B_PROTOCOL_REQUEST		0x8041
#define SELECT_B_PROTOCOL_CONFIRM		0x8141

#define LISTEN_REQ_CMD		0x05
#define LISTEN_REQ_SUB		0x80
#define LISTEN_CONF_CMD		0x05
#define LISTEN_CONF_SUB		0x81

/*----- CAPI commands -----*/

#define CAPI_ALERT					0x01
#define CAPI_CONNECT				0x02
#define CAPI_CONNECT_ACTIVE			0x03
#define CAPI_CONNECT_B3_ACTIVE	    0x83
#define CAPI_CONNECT_B3 			0x82
#define CAPI_CONNECT_B3_T90_ACTIVE  0x88
#define CAPI_DATA_B3				0x86
#define CAPI_DISCONNECT_B3		    0x84
#define CAPI_DISCONNECT 		    0x04
#define CAPI_FACILITY				0x80
#define CAPI_INFO				    0x08
#define CAPI_LISTEN				    0x05
#define CAPI_MANUFACTURER		    0xff
#define CAPI_RESET_B3				0x87
#define CAPI_SELECT_B_PROTOCOL	    0x41

/*----- CAPI subcommands -----*/

#define CAPI_REQ    0x80
#define CAPI_CONF   0x81
#define CAPI_IND    0x82
#define CAPI_RESP   0x83

//******************************************************************************
//******************************************************************************
//******************************************************************************
typedef struct _CALLING_PARTY_NUMBER
{
	BYTE nombertype;
	BYTE screening_indicator;
	BYTE callingpartynomber[10];
} CALLING_PARTY_NUMBER;

typedef struct _CALLING_PARTY_SUBADRESS
{
	BYTE subadresstype;
	BYTE info[10];
} CALLING_PARTY_SUBADRESS;

typedef struct _CALLED_PARTY_NUMBER
{
	BYTE type_of_number;
	BYTE calledpartynumber[5];
} CALLED_PARTY_NUMBER;

typedef struct _CALLED_PARTY_SUBADRESS
{
	BYTE _subadresstype;
	BYTE _info[10];
} CALLED_PARTY_SUBADRESS;
//******************************************************************************
//CAPI msg TYPEs
typedef struct _MSG_TYPE
{
	BYTE cmd;
	BYTE sub;
} MSG_TYPE;

typedef struct __B_PROTOCOL
{
	BYTE StructLen[1];
	WORD B1protocol;
	WORD B2protocol;
	WORD B3protocol;
	BYTE B1configuration[1];
	BYTE B2configuration[1];
	BYTE B3configuration[1];
	//BYTE GlobalConfiguration[1];
} _B_PROTOCOL;

typedef struct __ADDITIONAL_INFO
{
	BYTE StructLen[1];
	BYTE BChannelinformation[1];
	BYTE Keypadfacility[1];
	BYTE Useruserdata[1];
	BYTE Facilitydataarray[1];
	BYTE SendingComplete[1];
} _ADDITIONAL_INFO;

typedef struct __ALERT_REQ
{
	DWORD PLCI;
	BYTE StructLen[1];
	BYTE BChannelinformation[1];
	BYTE Keypadfacility[1];
	BYTE Useruserdata[1];
	BYTE Facilitydataarray[1];
	BYTE SendingComplete[1];
} _ALERT_REQ;

typedef struct __CONNECT_B3_RESP
{
	DWORD NCCI;
	WORD Reject;
	BYTE structsNCPI[1];
} _CONNECT_B3_RESP;

typedef struct __CONNECT_B3_IND
{
	DWORD NCCI;
	BYTE structsNCPI[1];
} _CONNECT_B3_IND;

typedef struct __CONNECT_B3_ACTIVE_IND
{
	DWORD NCCI;
	BYTE structs[1];
} _CONNECT_B3_ACTIVE_IND;

typedef struct __CONNECT_B3_ACTIVE_RESP
{
	DWORD NCCI;
} _CONNECT_B3_ACTIVE_RESP;

typedef struct __CONNECT_ACTIVE_RESP
{
	DWORD PLCI;
} _CONNECT_ACTIVE_RESP;

typedef struct __CONNECT_ACTIVE_IND
{
	DWORD PLCI;
	BYTE structs[1];
} _CONNECT_ACTIVE_IND;

typedef struct _CONNECT_IND
{
	DWORD PLCI;
	WORD CIP_VALUE;
	BYTE structs[1];
} CONNECT_IND;

typedef struct _CONNECT_RESP
{
	DWORD PLCI;
	WORD REJECT;
/*	_B_PROTOCOL bprotocol;
	BYTE ConnectedNumber[1];
	BYTE ConnectedSubaddress[1];
	BYTE LLC[1];
	_ADDITIONAL_INFO add_info;*/
	BYTE structs[22]; //16
} CONNECT_RESP;

typedef struct __DATA_B3_REQ
{
	DWORD NCCI;
	DWORD Data;
	WORD DataLen;
	WORD DataHandle;
	WORD Flags;
	unsigned __int64 Data64;
} _DATA_B3_REQ;

typedef struct __DATA_B3_CONF
{
	DWORD NCCI;
	WORD DataHandle;
	WORD Info;
} _DATA_B3_CONF;

typedef struct __DATA_B3_IND
{
	DWORD NCCI;
	DWORD Data;
	WORD DataLen;
	WORD DataHandle;
	WORD Flags;
	unsigned __int64 Data64;
} _DATA_B3_IND;

typedef struct __DATA_B3_RESP
{
	DWORD NCCI;
	WORD DataHandle;
} _DATA_B3_RESP;

typedef struct _DISCONNECT_IND
{
	DWORD PLCI;
	WORD REASON;
} DISCONNECT_IND;

typedef struct _DISCONNECT_RESP
{
	DWORD PLCI;
} DISCONNECT_RESP;


typedef struct _LISTEN_REQ
{
	DWORD Controller;
	DWORD Info_Mask;
	DWORD CIP_Mask;
	DWORD CIP_Mask2;
	CALLING_PARTY_NUMBER CallingPartyNomber;
	CALLING_PARTY_SUBADRESS CallingPartySubaddress;
} LISTEN_REQ;

typedef struct _LISTEN_CONF
{
	DWORD Controller;
	WORD Info;
} LISTEN_CONF;

typedef struct _MSG_HEADER
{
	WORD Totallength;
	WORD Applid;
	BYTE Command;
	BYTE SubCommand;
	WORD MessageNumber;
} CAPI_MSG_HEADER;

typedef union _MSG_PARAMETERS
{
	_ALERT_REQ				alert_req;
	_CONNECT_B3_ACTIVE_IND	connect_b3_active_ind;
	_CONNECT_B3_ACTIVE_RESP connect_b3_active_resp;
	_CONNECT_B3_RESP		connect_b3_resp;
	_CONNECT_B3_IND			connect_b3_ind;
	_CONNECT_ACTIVE_RESP	connect_active_resp;
	_CONNECT_ACTIVE_IND		connect_active_ind;
	_DATA_B3_REQ			data_b3_req;
	_DATA_B3_CONF			data_b3_conf;
	_DATA_B3_IND			data_b3_ind;
	_DATA_B3_RESP			data_b3_resp;
	LISTEN_REQ				listen_req;
	LISTEN_CONF				listen_conf;
	CONNECT_IND				connect_ind;
	CONNECT_RESP			connect_resp;
	DISCONNECT_IND			disconnect_ind;
	DISCONNECT_RESP			disconnect_resp;
} PARAMETERS;

typedef struct _API_MSG
{
	CAPI_MSG_HEADER header;
	PARAMETERS		parameters;
} CAPI_MSG;
//******************************************************************************

//******************************************************************************
//Класс который позволяет определять номера звонков входящих на 
//ISDN телефон
class CCapiFunc  
{
public:
	CCapiFunc();
	~CCapiFunc();
	int Init();
	int SetModeCapi(DWORD mode);
	void Handle_CAPI_Msg();
	void Handle_Indication(void);
	void Handle_Confirmation(void);
	void GetCalledPartyNumberStruct(CAPI_MSG *msg);
	void GetCallingPartyNumberStruct(CAPI_MSG *msg);
	void CONNECT_RESP(CAPI_MSG * msgIn);
	void DISCONNECT_RESP();
	int Close();
	int FreeCapi32();
	CCapiFunc * m_pThisObject;
	char CallingPartyNomber[64];
	char CalledPartyNomber[64];
	DWORD ApplID;
	DWORD MessageBufferSize;
	DWORD maxBDataLen;
	DWORD maxBDataBlocks;
	DWORD maxLogicalConnection;
	DWORD NCCI;
	WORD Reject;

protected:
	CAPIREGISTER*			pCAPI_REGISTER;
	CAPIRELEASE*			pCAPI_RELEASE;
	CAPIPUTMESSAGE*			pCAPI_PUT_MESSAGE;
	CAPIGETMESSAGE*			pCAPI_GET_MESSAGE;
	CAPIWAITFORSIGNAL*		pCAPI_WAIT_FOR_SIGNAL;
	CAPIGETMANUFACTURER*	pCAPI_GET_MANUFACTURER;
	CAPIGETVERSION*			pCAPI_GET_VERSION;
	CAPIGETSERIALNUMBER*	pCAPI_GET_SERIAL_NUMBER;
	CAPIGETPROFILE*			pCAPI_GET_PROFILE;
	CAPIINSTALLED*			pCAPI_INSTALLED;
	HINSTANCE h;
	PVOID * ppCAPIMessage;

	DWORD CAPI_REGISTER( DWORD MessageBufferSize,
						 DWORD maxLogicalConnection,
						 DWORD maxBDataBlocks,
						 DWORD maxBDataLen,
						 DWORD * pApplID );

	DWORD CAPI_RELEASE( DWORD ApplID );

	DWORD CAPI_PUT_MESSAGE( DWORD ApplID, 
							PVOID pCAPIMessage);

	DWORD CAPI_GET_MESSAGE( DWORD ApplID,
							PVOID * ppCAPIMessage);

	DWORD CAPI_WAIT_FOR_SIGNAL( DWORD ApplID);

	void CAPI_GET_MANUFACTURER( char * SzBuffer);

	DWORD CAPI_GET_VERSION( DWORD * pCAPIMajor,
							DWORD * pCAPIMinor,
							DWORD * pManufacturerMajor,
							DWORD * pManufacturerMinor );

	DWORD CAPI_GET_SERIAL_NUMBER( char * SzBuffer);

	DWORD CAPI_GET_PROFILE( PVOID SzBuffer, 
							DWORD CtrlNr );

	DWORD CAPI_INSTALLED(void);
	int LoadCapi32();
//	int FreeCapi32();
private:
	char comand[30];
	char * Decode_Info (unsigned int Info);
	char * Decode_Full_Command(CAPI_MSG * msg);
	char * Decode_Sub (unsigned char Sub);
	char * Decode_Command (unsigned char Command);
};
