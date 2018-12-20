#include <windows.h>
#include <stdio.h>
#include "isdnfinder.h"
#include "CapiFunc.h"

CCapiFunc::CCapiFunc()
{

}

CCapiFunc::~CCapiFunc()
{
	Close();
	FreeCapi32();
}

int CCapiFunc::LoadCapi32()
{
	int error;
	h=LoadLibrary("CAPI2032.DLL");
	if (!h)
	{
		error = GetLastError();
		return error;
	}
	return 0;
}

int CCapiFunc::FreeCapi32()
{
	int error;

	if ((error = FreeLibrary(h)) == 0)
	{
		error = GetLastError();
		return error;
	}
	
	return 0;
}

int CCapiFunc::Close()
{
	DWORD result;

	if ((result = CAPI_RELEASE(ApplID)) != 0)
	{
		//toLog(Decode_Info(result));
	}
	else
	{
		printf("%s\n", Decode_Info(result));
	}

	return 0;
}

int CCapiFunc::Init()
{
	int error;
	if ( error = LoadCapi32() != 0) 
	{ 
		//ErrorShow("CAPI NOT Loaded.\n");
		return error; 
	} 

	if ((error = CAPI_INSTALLED()) != 0)
	{
		ErrorShow("CAPI NOT Installed.\n");
		return error;
	}
	else
	{
		maxLogicalConnection = 2;
		maxBDataBlocks = 7;
		maxBDataLen = 2048;
		MessageBufferSize = (1024 + (1024 * maxLogicalConnection)); 
		if ((error = CAPI_REGISTER( MessageBufferSize,
									maxLogicalConnection,
									maxBDataBlocks,
									maxBDataLen,
									&ApplID )) != 0)
		{
			ErrorShow("CAPI NOT Registered.\n");
			return error;
		}
	}

	if ((ppCAPIMessage = (PVOID*) malloc(1 * sizeof(DWORD))) == NULL)
	{
		ErrorShow("not enougth space for pCAPIMessage\n") ;
		return 1;
	}

	NCCI = 0;

	return 0;
}

int CCapiFunc::SetModeCapi(DWORD mode)
{
	DWORD result;

	switch (mode)
	{
	case 1:
		{
			CAPI_MSG Listen_Req_Msg;

			Listen_Req_Msg.header.Totallength = sizeof(CAPI_MSG);
			Listen_Req_Msg.header.Applid = (WORD)ApplID;
			Listen_Req_Msg.header.Command = LISTEN_REQ_CMD;
			Listen_Req_Msg.header.SubCommand = LISTEN_REQ_SUB;
			Listen_Req_Msg.header.MessageNumber = 0x1;
			Listen_Req_Msg.parameters.listen_req.Controller = 0x0081;//0x1;
			Listen_Req_Msg.parameters.listen_req.Info_Mask = 0x7f; //0x00000000;
			Listen_Req_Msg.parameters.listen_req.CIP_Mask = 0x30012 ; //0x1FFF03FF;
			Listen_Req_Msg.parameters.listen_req.CIP_Mask2 = 0x00000000;
			Listen_Req_Msg.parameters.listen_req.CallingPartyNomber.callingpartynomber[0] = 0x0;
			Listen_Req_Msg.parameters.listen_req.CallingPartyNomber.nombertype = 0x0;
			Listen_Req_Msg.parameters.listen_req.CallingPartyNomber.screening_indicator = 0x0;
			Listen_Req_Msg.parameters.listen_req.CallingPartySubaddress.subadresstype = 0x0;
			result = CAPI_PUT_MESSAGE(ApplID, &Listen_Req_Msg);
			if (result != 0)
			{
				ErrorShow("LISTEN_REQ fault!!!");
			}
			else
				printf("->LISTEN_REQ\n");

			break;
		}
	default:
		return 1;
	}

	return 0;
}

void CCapiFunc::Handle_CAPI_Msg()
{
	DWORD result;
	CAPI_MSG * msgIn = NULL;

	if ((result = CAPI_WAIT_FOR_SIGNAL(ApplID)) == 0)
	{
		result = CAPI_GET_MESSAGE(ApplID, ppCAPIMessage);
		msgIn = (CAPI_MSG *)(*ppCAPIMessage) ;
		switch (result) 
		{
		case 0x0000:	       /*----- a message has been read -----*/
			switch (msgIn->header.SubCommand) 
			{
			case CAPI_CONF:
				Handle_Confirmation();
				break;
			case CAPI_IND:
				Handle_Indication();
				break;
			default:	   /*----- neither indication nor confirmation ???? -----*/
				printf("Error: Unknown subcommand in function Handle_CAPI_Msg.\n");
				return;
			}
			break;
		case 0x1104:
			return;	/*----- messagequeue is empty -----*/
		default:
			printf("Error: CAPI_GET_MESSAGE returns result != 0 in function Handle_CAPI_Msg\n");
			return;
		}
	}
	else
	{
		char buf[512];
		buf[0] = 0;
		strcpy(buf,"Wait for signal return non zero in function Handle_CAPI_Msg, Info : ");
		strcat(buf,Decode_Info(result));
		printf("%s\n",buf);
	}
}

void CCapiFunc::Handle_Confirmation()
{
	CAPI_MSG * msgIn = NULL;

	msgIn = (CAPI_MSG *)(*ppCAPIMessage);
	switch (msgIn->header.Command) 
	{
	case CAPI_CONNECT:
		{
			return;
		}

	case CAPI_CONNECT_B3:
		{
			return;
		}

	case CAPI_DISCONNECT:
		{
			return;
		}

	case CAPI_DISCONNECT_B3:
		{
			return;
		}

	case CAPI_DATA_B3:
		{
			return;
		}
	case CAPI_LISTEN:
		{
			if (msgIn->parameters.listen_conf.Info > 0x00FF)
			{
				printf("Error: Info != 0 in LISTEN_CONF in function Handle_Confirmation\n");
			}
			else
				printf("<-LISTEN_CONF\n");
			return;
		}
	case CAPI_INFO:
		{
		    return;
		}
	case CAPI_ALERT:
		{
			return;
		}
	default:
		{
			ErrorShow("Error: Confirmation not handled in function Handle_Confirmation");
			return;
		}
	}
}

void CCapiFunc::Handle_Indication()
{
	CAPI_MSG * msgIn = NULL;

	msgIn = (CAPI_MSG *)(*ppCAPIMessage);
    switch (msgIn->header.Command) 
	{
    case CAPI_CONNECT:
		{
			printf("<-CONNECT_IND\n");
			GetCallingPartyNumberStruct(msgIn);
			GetCalledPartyNumberStruct(msgIn);
			CONNECT_RESP(msgIn);
			return;
		}
    case CAPI_CONNECT_ACTIVE:
		{
			return;
		}
    case CAPI_CONNECT_B3:
		{
			return;
		}
    case CAPI_CONNECT_B3_ACTIVE:
		{
			return;
		}
    case CAPI_DISCONNECT_B3:
		{

			return;
		}
    case CAPI_DISCONNECT:
		{
			printf("<-DISCONNECT_RESP\n");
			DISCONNECT_RESP();
			return;
		}
    case CAPI_DATA_B3:
		{
			return;
		}
    case CAPI_INFO:
		{
			return;
		}
    default:
		{
			ErrorShow("Error: Indication not handled in function Handle_Indication");
			return;
		}
    }
}

void CCapiFunc::GetCalledPartyNumberStruct(CAPI_MSG *msg)
{
	BYTE * pMsgB;
	unsigned lengthstructCdPN = 0;
	unsigned lengthstructCgPN = 0;
	unsigned lengthCdPN = 0;
	unsigned lengthCgPN = 0;
	unsigned typeCdPN = 0;
	unsigned typeCgPN = 0;
	unsigned present_and_screening_indicator = 0;

	pMsgB = (BYTE*)msg;

	lengthstructCdPN = *(pMsgB + 14);
	lengthstructCgPN = *(pMsgB + 15 + lengthstructCdPN);
	lengthCdPN = lengthstructCdPN - 1;
	lengthCgPN = lengthstructCgPN - 2;
	typeCdPN = *(pMsgB + 15);
	typeCgPN = *(pMsgB + 16 + lengthstructCdPN);
	present_and_screening_indicator = *(pMsgB + 17 + lengthstructCdPN);

	if (lengthCdPN != 0)
	{
		/*if ( (CalledPartyNomber = (char*)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY, lengthCdPN + 1)) == NULL)
		{
			ErrorShow("Невозможно выделить память, GetCalledPartyNumberStruct() \n");
			return;
		}*/
		
		memcpy((PVOID)CalledPartyNomber, ((PVOID)(pMsgB + 16)), lengthCdPN);
		*((char *)CalledPartyNomber + lengthCgPN ) = 0;
	}
	else
		ZeroMemory((PVOID)CalledPartyNomber, 64);
}

void CCapiFunc::GetCallingPartyNumberStruct(CAPI_MSG *msg)
{
	BYTE * pMsgB;
	unsigned lengthstructCdPN = 0;
	unsigned lengthstructCgPN = 0;
	unsigned lengthCdPN = 0;
	unsigned lengthCgPN = 0;
	unsigned typeCdPN = 0;
	unsigned typeCgPN = 0;
	unsigned present_and_screening_indicator = 0;

	pMsgB = (BYTE*)msg;

	lengthstructCdPN = *(pMsgB + 14);
	lengthstructCgPN = *(pMsgB + 15 + lengthstructCdPN);
	lengthCdPN = lengthstructCdPN - 1;
	lengthCgPN = lengthstructCgPN - 2;
	typeCdPN = *(pMsgB + 15);
	typeCgPN = *(pMsgB + 16 + lengthstructCdPN);
	present_and_screening_indicator = *(pMsgB + 17 + lengthstructCdPN);

	if (lengthCgPN != 0)
	{
		/*if ( (CallingPartyNomber = (char*)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY, lengthCgPN + 1)) == NULL)
		{
			ErrorShow("Невозможно выделить память, GetCallingPartyNumberStruct()\n");
			return;
		}*/
		
		memcpy((PVOID)CallingPartyNomber, ((PVOID)(pMsgB + 18 + lengthstructCdPN)), lengthCgPN);
		*((char *)CallingPartyNomber + lengthCgPN ) = 0;
	}
	else
		ZeroMemory((PVOID)CallingPartyNomber, 64); 
}

void CCapiFunc::CONNECT_RESP(CAPI_MSG * msgIn)
{
	CAPI_MSG msgOut;
	DWORD result;

	msgOut.parameters.connect_resp.PLCI = msgIn->parameters.connect_ind.PLCI;
	msgOut.parameters.connect_resp.REJECT = Reject;//0x0001;
	msgOut.parameters.connect_resp.structs[0] = 9; //bprotocol

	msgOut.parameters.connect_resp.structs[1] = 1;
	msgOut.parameters.connect_resp.structs[2] = 0;
	msgOut.parameters.connect_resp.structs[3] = 1;
	msgOut.parameters.connect_resp.structs[4] = 0;
	msgOut.parameters.connect_resp.structs[5] = 0;
	msgOut.parameters.connect_resp.structs[6] = 0;
	msgOut.parameters.connect_resp.structs[7] = 0;
	msgOut.parameters.connect_resp.structs[8] = 0;
	msgOut.parameters.connect_resp.structs[9] = 0;

	msgOut.parameters.connect_resp.structs[10] = 0;//ConnectedNumber
	msgOut.parameters.connect_resp.structs[11] = 0; //ConnectedSubaddress
	msgOut.parameters.connect_resp.structs[12] = 0; //LLC

	msgOut.parameters.connect_resp.structs[13] = 4; //add_info

	msgOut.parameters.connect_resp.structs[14] = 2; //B Channel Information
	msgOut.parameters.connect_resp.structs[15] = 0;
	msgOut.parameters.connect_resp.structs[16] = 0;
	msgOut.parameters.connect_resp.structs[17] = 0;
	msgOut.parameters.connect_resp.structs[18] = 0;

	msgOut.parameters.connect_resp.structs[19] = 0;
	msgOut.parameters.connect_resp.structs[20] = 0;
	msgOut.parameters.connect_resp.structs[21] = 0;

	msgOut.header.Applid = (unsigned short)ApplID;
	msgOut.header.Command = 0x02;
	msgOut.header.SubCommand = 0x83;
	msgOut.header.MessageNumber = 0;
	msgOut.header.Totallength = 36;

	result = CAPI_PUT_MESSAGE(ApplID, &msgOut);
	if (result != 0)
	{
		char buf[512]; buf[0] = 0;
		strcpy(buf,"CONNECT_RESP error. ");
		strcat(buf,Decode_Info(result));
		printf("%s\n", buf);
	}
	else
	{
		char buf[512]; buf[0] = 0;
		strcpy(buf,"-> CONNECT_RESP, PLCI= ");
		strcat(buf,Decode_Info(result));
		printf("%s\n", buf);
	}
}

void CCapiFunc::DISCONNECT_RESP()
{
	CAPI_MSG msgOut, *msgIn;;
	DWORD result;

	msgIn = (CAPI_MSG *)(*ppCAPIMessage);
	msgOut.parameters.disconnect_resp.PLCI = msgIn->parameters.connect_ind.PLCI;
	msgOut.header.Applid = (unsigned short)ApplID;
	msgOut.header.Command = 0x04;
	msgOut.header.SubCommand = 0x83;
	msgOut.header.MessageNumber = 3;
	msgOut.header.Totallength = 12;
	result = CAPI_PUT_MESSAGE(ApplID, &msgOut);
	if (result != 0)
	{
		printf("DISCONNECT_RESP error.\n");
	}
	else
	{
		printf("-> DISCONNECT_RESP\n");
	}
}

//*********************************************************************************
//Функции импортируемые из библиотеки CAPI32.DLL
//*********************************************************************************
DWORD CCapiFunc::CAPI_REGISTER( DWORD MessageBufferSize,
								DWORD maxLogicalConnection,
								DWORD maxBDataBlocks,
								DWORD maxBDataLen,
								DWORD * pApplID )
{
	DWORD result;
	pCAPI_REGISTER = (CAPIREGISTER*)GetProcAddress((HMODULE) h, "CAPI_REGISTER");
	if (!pCAPI_REGISTER)
	{
		result = GetLastError();
		return result;
	}
	result = pCAPI_REGISTER( MessageBufferSize,
						 	 maxLogicalConnection,
							 maxBDataBlocks,
							 maxBDataLen,
							 pApplID );
	return result;
}

DWORD CCapiFunc::CAPI_RELEASE( DWORD ApplID )
{
	DWORD result;
	pCAPI_RELEASE = (CAPIRELEASE*)GetProcAddress((HMODULE) h, "CAPI_RELEASE");
	if (!pCAPI_RELEASE)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_RELEASE\n");
		result = GetLastError();
		return result;
	}
	result = pCAPI_RELEASE(ApplID);
	return result;
}

DWORD CCapiFunc::CAPI_PUT_MESSAGE( DWORD ApplID, PVOID pCAPIMessage)
{
	DWORD result;
	pCAPI_PUT_MESSAGE = (CAPIPUTMESSAGE*)GetProcAddress((HMODULE) h, "CAPI_PUT_MESSAGE");
	if (!pCAPI_PUT_MESSAGE)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_PUT_MESSAGE\n");
		result = GetLastError();
		return result;
	}
	result = pCAPI_PUT_MESSAGE(ApplID, pCAPIMessage);
	return result;
}

DWORD CCapiFunc::CAPI_GET_MESSAGE( DWORD ApplID, PVOID * ppCAPIMessage)
{
	DWORD result;
	pCAPI_GET_MESSAGE = (CAPIGETMESSAGE*)GetProcAddress((HMODULE) h, "CAPI_GET_MESSAGE");
	if (!pCAPI_GET_MESSAGE)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_GET_MESSAGE\n");
		result = GetLastError();
		return result;
	}
	result = pCAPI_GET_MESSAGE(ApplID, ppCAPIMessage);
	return result;
}

DWORD CCapiFunc::CAPI_WAIT_FOR_SIGNAL( DWORD ApplID)
{
	DWORD result;
	pCAPI_WAIT_FOR_SIGNAL = 
		(CAPIWAITFORSIGNAL*)GetProcAddress((HMODULE) h, "CAPI_WAIT_FOR_SIGNAL");
	if (!pCAPI_WAIT_FOR_SIGNAL)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_WAIT_FOR_SIGNAL\n");
		result = GetLastError();
		return result;
	}
	result = pCAPI_WAIT_FOR_SIGNAL(ApplID);
	return result;
}

void CCapiFunc::CAPI_GET_MANUFACTURER(char * SzBuffer)
{
	pCAPI_GET_MANUFACTURER = 
		(CAPIGETMANUFACTURER*)GetProcAddress((HMODULE) h, "CAPI_GET_MANUFACTURER");
	if (!pCAPI_GET_MANUFACTURER)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_GET_MANUFACTURER\n");
		//return 1;
	}
	pCAPI_GET_MANUFACTURER(SzBuffer);
}

DWORD CCapiFunc::CAPI_INSTALLED(void)
{
	DWORD result;

	pCAPI_INSTALLED = (CAPIINSTALLED*)GetProcAddress((HMODULE)h,"CAPI_INSTALLED");
	if (!pCAPI_INSTALLED)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_INSTALLED\n");
		result = GetLastError();
		return result;
	}
	result = pCAPI_INSTALLED();
	return result;
}

DWORD CCapiFunc::CAPI_GET_SERIAL_NUMBER(char * SzBuffer)
{
	DWORD result;

	pCAPI_GET_SERIAL_NUMBER = (CAPIGETSERIALNUMBER*)GetProcAddress((HMODULE) h,"CAPI_GET_SERIAL_NUMBER");	
	if (!pCAPI_GET_SERIAL_NUMBER)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_GET_SERIAL_NUMBER\n");
		result = GetLastError();
		return result;
	}
	result = pCAPI_GET_SERIAL_NUMBER(SzBuffer);
	return result;
}

DWORD CCapiFunc::CAPI_GET_PROFILE( PVOID SzBuffer, DWORD CtrlNr)
{
	DWORD result;
	
	pCAPI_GET_PROFILE = (CAPIGETPROFILE*)GetProcAddress((HMODULE) h,"CAPI_GET_PROFILE");
	if (!pCAPI_GET_PROFILE)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_GET_PROFILE\n");
		result = GetLastError();
		return result;
	}
	result = pCAPI_GET_PROFILE(SzBuffer, CtrlNr);
	return result;
}

DWORD CCapiFunc::CAPI_GET_VERSION( DWORD * pCAPIMajor,
						DWORD * pCAPIMinor,
						DWORD * pManufacturerMajor,
						DWORD * pManufacturerMinor )
{
	DWORD result;

	pCAPI_GET_VERSION = (CAPIGETVERSION*)GetProcAddress((HMODULE)h,"CAPI_GET_VERSION");
	if (!pCAPI_GET_VERSION)
	{
		//printf("ERROR! in dll\nnot find function  CAPI_GET_VERSION\n");
		result = GetLastError();
		return result;
	}
	result = pCAPI_GET_VERSION ( pCAPIMajor, 
							     pCAPIMinor, 
								 pManufacturerMajor, 
								 pManufacturerMinor);
	return result;
}

//*********************************************************************************
//Функции декодирования сообщений и результатов
char * CCapiFunc::Decode_Command(unsigned char Command)
{
    switch (Command) 
	{
	case 0x01: return "ALERT";
	case 0x02: return "CONNECT";
	case 0x03: return "CONNECT_ACTIVE";
	case 0x04: return "DISCONNECT";
	case 0x05: return "LISTEN";
	case 0x08: return "INFO";
	case 0x41: return "SELECT_B_PROTOCOL";
	case 0x80: return "FACILITY";
	case 0x82: return "CONNECT_B3";
	case 0x83: return "CONNECT_B3_ACTIVE";
	case 0x84: return "DISCONNECT_B3";
	case 0x86: return "DATA_B3";
	case 0x87: return "RESET_B3";
	case 0x88: return "CONNECT_B3_T90_ACTIVE";
	case 0xff: return "MANUFACTURER";
    }
    return "Error: Command undefined in function Decode_Command";
}

char * CCapiFunc::Decode_Sub(unsigned char Sub)
{
    switch (Sub) 
	{
	case 0x80: return "REQ";    /*----- Request -----*/
	case 0x81: return "CONF";   /*----- Confirmation -----*/
	case 0x82: return "IND";    /*----- Indication -----*/
	case 0x83: return "RESP";   /*----- Response -----*/
    }
    return "Error: Subcommand undefined in function Decode_Sub";
}

char * CCapiFunc::Decode_Full_Command(CAPI_MSG *msg)
{
	MSG_TYPE msg_type;

	msg_type.cmd = msg->header.Command ;
	msg_type.sub = msg->header.SubCommand;
	strcpy(comand, Decode_Command((unsigned char)msg_type.cmd));
	strcat(comand, "_");
	strcat(comand, Decode_Sub((unsigned char)msg_type.sub));
	return comand;
}

char * CCapiFunc::Decode_Info(unsigned int Info)
{
    switch (Info) 
	{
/*-- informative values (corresponding message was processed) -----*/
	case 0x0001: return "NCPI not supported by current protocol, NCPI ignored";
	case 0x0002: return "Flags not supported by current protocol, flags ignored";
	case 0x0003: return "Alert already sent by another application";

/*-- error information concerning CAPI_REGISTER -----*/
	case 0x1001: return "Too many applications";
	case 0x1002: return "Logical block size to small, must be at least 128 Bytes";
	case 0x1003: return "Buffer exceeds 64 kByte";
	case 0x1004: return "Message buffer size too small, must be at least 1024 Bytes";
	case 0x1005: return "Max. number of logical connections not supported";
	case 0x1006: return "Reserved";
	case 0x1007: return "The message could not be accepted because of an internal busy condition";
	case 0x1008: return "OS resource error (no memory ?)";
	case 0x1009: return "CAPI not installed";
	case 0x100A: return "Controller does not support external equipment";
	case 0x100B: return "Controller does only support external equipment";

/*-- error information concerning message exchange functions -----*/
	case 0x1101: return "Illegal application number";
	case 0x1102: return "Illegal command or subcommand or message length less than 12 bytes";
	case 0x1103: return "The message could not be accepted because of a queue full condition !! The error code does not imply that CAPI cannot receive messages directed to another controller, PLCI or NCCI";
	case 0x1104: return "Queue is empty";
	case 0x1105: return "Queue overflow, a message was lost !! This indicates a configuration error. The only recovery from this error is to perform a CAPI_RELEASE";
	case 0x1106: return "Unknown notification parameter";
	case 0x1107: return "The Message could not be accepted because of an internal busy condition";
	case 0x1108: return "OS Resource error (no memory ?)";
	case 0x1109: return "CAPI not installed";
	case 0x110A: return "Controller does not support external equipment";
	case 0x110B: return "Controller does only support external equipment";

/*-- error information concerning resource / coding problems -----*/
	case 0x2001: return "Message not supported in current state";
	case 0x2002: return "Illegal Controller / PLCI / NCCI";
	case 0x2003: return "Out of PLCI";
	case 0x2004: return "Out of NCCI";
	case 0x2005: return "Out of LISTEN";
	case 0x2006: return "Out of FAX resources (protocol T.30)";
	case 0x2007: return "Illegal message parameter coding";

/*-- error information concerning requested services  -----*/
	case 0x3001: return "B1 protocol not supported";
	case 0x3002: return "B2 protocol not supported";
	case 0x3003: return "B3 protocol not supported";
	case 0x3004: return "B1 protocol parameter not supported";
	case 0x3005: return "B2 protocol parameter not supported";
	case 0x3006: return "B3 protocol parameter not supported";
	case 0x3007: return "B protocol combination not supported";
	case 0x3008: return "NCPI not supported";
	case 0x3009: return "CIP Value unknown";
	case 0x300A: return "Flags not supported (reserved bits)";
	case 0x300B: return "Facility not supported";
	case 0x300C: return "Data length not supported by current protocol";
	case 0x300D: return "Reset procedure not supported by current protocol";

/*-- informations about the clearing of a physical connection -----*/
	case 0x3301: return "Protocol error layer 1 (broken line or B-channel removed by signalling protocol)";
	case 0x3302: return "Protocol error layer 2";
	case 0x3303: return "Protocol error layer 3";
	case 0x3304: return "Another application got that call";
/*-- T.30 specific reasons -----*/
	case 0x3311: return "Connecting not successful (remote station is no FAX G3 machine)";
	case 0x3312: return "Connecting not successful (training error)";
	case 0x3313: return "Disconnected before transfer (remote station does not support transfer mode, e.g. resolution)";
	case 0x3314: return "Disconnected during transfer (remote abort)";
	case 0x3315: return "Disconnected during transfer (remote procedure error, e.g. unsuccessful repetition of T.30 commands)";
	case 0x3316: return "Disconnected during transfer (local tx data underrun)";
	case 0x3317: return "Disconnected during transfer (local rx data overflow)";
	case 0x3318: return "Disconnected during transfer (local abort)";
	case 0x3319: return "Illegal parameter coding (e.g. SFF coding error)";

/*-- disconnect causes from the network according to ETS 300 102-1/Q.931 -----*/
	case 0x3481: return "Unallocated (unassigned) number";
	case 0x3482: return "No route to specified transit network";
	case 0x3483: return "No route to destination";
	case 0x3486: return "Channel unacceptable";
	case 0x3487: return "Call awarded and being delivered in an established channel";
	case 0x3490: return "Normal call clearing";
	case 0x3491: return "User busy";
	case 0x3492: return "No user responding";
	case 0x3493: return "No answer from user (user alerted)";
	case 0x3495: return "Call rejected";
	case 0x3496: return "Number changed";
	case 0x349A: return "Non-selected user clearing";
	case 0x349B: return "Destination out of order";
	case 0x349C: return "Invalid number format";
	case 0x349D: return "Facility rejected";
	case 0x349E: return "Response to STATUS ENQUIRY";
	case 0x349F: return "Normal, unspecified";
	case 0x34A2: return "No circuit / channel available";
	case 0x34A6: return "Network out of order";
	case 0x34A9: return "Temporary failure";
	case 0x34AA: return "Switching equipment congestion";
	case 0x34AB: return "Access information discarded";
	case 0x34AC: return "Requested circuit / channel not available";
	case 0x34AF: return "Resources unavailable, unspecified";
	case 0x34B1: return "Quality of service unavailable";
	case 0x34B2: return "Requested facility not subscribed";
	case 0x34B9: return "Bearer capability not authorized";
	case 0x34BA: return "Bearer capability not presently available";
	case 0x34BF: return "Service or option not available, unspecified";
	case 0x34C1: return "Bearer capability not implemented";
	case 0x34C2: return "Channel type not implemented";
	case 0x34C5: return "Requested facility not implemented";
	case 0x34C6: return "Only restricted digital information bearer capability is available";
	case 0x34CF: return "Service or option not implemented, unspecified";
	case 0x34D1: return "Invalid call reference value";
	case 0x34D2: return "Identified channel does not exist";
	case 0x34D3: return "A suspended call exists, but this call identity does not";
	case 0x34D4: return "Call identity in use";
	case 0x34D5: return "No call suspended";
	case 0x34D6: return "Call having the requested call identity has been cleared";
	case 0x34D8: return "Incompatible destination";
	case 0x34DB: return "Invalid transit network selection";
	case 0x34DF: return "Invalid message, unspecified";
	case 0x34E0: return "Mandatory information element is missing";
	case 0x34E1: return "Message type non-existent or not implemented";
	case 0x34E2: return "Message not compatible with call state or message type non-existent or not implemented";
	case 0x34E3: return "Information element non-existent or not implemented";
	case 0x34E4: return "Invalid information element contents";
	case 0x34E5: return "Message not compatible with call state";
	case 0x34E6: return "Recovery on timer expiry";
	case 0x34EF: return "Protocol error, unspecified";
	case 0x34FF: return "Interworking, unspecified";

	default: return "No additional information";
    }
}

//*********************************************************************************