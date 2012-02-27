/*
 * Calendar Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <glib.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>

#include "cals-typedef.h"
#include "cals-ical.h"
#include "cals-ical-codec.h"
#include "cals-utils.h"
#include "cals-db.h"
#include "cals-internal.h"
#include "cals-struct.h"


#define VCALENDAR_HEADER "BEGIN:VCALENDAR"
#define VNOTE_HEADER "BEGIN:VNOTE"

#define MAX_BUFFER_SIZE 200


/****************************************************************************************************/
/*						  GLOBAL VARIABLE DECLARATION AND INITIALIZATION							*/
/****************************************************************************************************/
#define MAX_CAL_TYPE_NAME 50

/* Type */
const char *pszCalTypeList[] =
{
	"AALARM",
	"ACTION",
	"ATTACH",
	"ATTENDEE",
	"BEGIN",
	"CALSCALE",
	"CATEGORIES",
	"CLASS",
	"COMMENT",
	"COMPLETED",
	"CONTACT",
	"CREATED",
	"DALARM",
	"DAYLIGHT",
	"DCREATED",
	"DESCRIPTION",
	"DTEND",
	"DTSTAMP",
	"DTSTART",
	"DUE",
	"DURATION",
	"END",
	"EXDATE",
	"EXRULE",
	"FREEBUSY",
	"GEO",
	"LAST-MODIFIED",
	"LOCATION",
	"MALARM",
	"METHOD",
	"ORGANIZER",
	"PALARM",
	"PERCENT-COMPLETE",
	"PRIORITY",
	"PRODID",
	"RDATE",
	"RECURRENCE-ID",
	"RELATED-TO",
	"REPEAT",
	"REQUEST-STATUS",
	"RESOURCES",
	"RNUM",
	"RRULE",
	"SEQUENCE",
	"STANDARD",
	"STATUS",
	"SUMMARY",
	"TRANSP",
	"TRIGGER",
	"TZ",
	"TZID",
	"TZNAME",
	"TZOFFSETFROM",
	"TZOFFSETTO",
	"TZURL",
	"URL",
	"UID",
	"VALARM",
	"VCALENDAR",
	"VERSION",
	"VEVENT",
	"VFREEBUSY",
	"VJOURNAL",
	"VTIMEZONE",
	"VTODO",
	"X-FUNAMBOL-ALLDAY"
};

/* Parameter */
static const char *pszCalParamList[] =
{
	"ALTREP",
	"CHARSET",
	"CN",
	"CONTEXT",
	"CUTYPE",
	"DELEGATED-FROM",
	"DELEGATED-TO",
	"DIR",
	"ENCODING",
	"EXPECT",
	"FBTYPE",
	"FMTYPE",
	"LANGUAGE",
	"MEMBER",
	"PARTSTAT",
	"RANGE",
	"RELATED",
	"RELTYPE",
	"ROLE",
	"RSVP",
	"SENT_BY",
	"STATUS",
	"TYPE",
	"TZID",
	"VALUE"
};

/* Cu type value */
static const ValueObj pCalCutypeValList[] =
{
	{"GROUP",		0x00000001},
	{"INDIVIDUAL",	0x00000002},
	{"RESOURCE",	0x00000004},
	{"ROOM",		0x00000008},
	{"UNKNOWN",		0x00000010}
};

/* Character set value */
static const ValueObj pCalCharsetValList[] =
{
	{"UTF-8",	0x00000001},
	{"UTF-16",	0x00000002}
};

/* Encoding value */
static const ValueObj pCalEncValList[] =
{
	{"B",		0x00000001},
	{"BASE64",	0x00000002},
	{"QUOTED-PRINTABLE", 0x00000004},
	{"7BIT",	0x00000008},
	{"8BIT",	0x00000010}
};

/* Fb type value */
static const ValueObj pCalFbtypeValList[] =
{
	{"BUSY",				0x00000001},
	{"BUSY_TENTATIVE",		0x00000002},
	{"BUSY_UNAVAILABLE",	0x00000004},
	{"FREE",				0x00000008}
};

/* Partstat value */
static const ValueObj pCalPartstatValList[] =
{
	{"ACCEPTED",		0x00000001},
	{"COMPLETED",		0x00000002},
	{"DELEGATED",		0x00000004},
	{"DECLINED",		0x00000008},
	{"IN_PROCESS",		0x00000010},
	{"NEED_ACTION",		0x00000020},
	{"TENTATIVE",		0x00000040}
};

/* Range value */
static const ValueObj pCalRangeValList[] =
{
	{"THISANDFUTURE",	0x00000001},
	{"THISANDPRIOR",	0x00000002}
};

/* Related value */
static const ValueObj pCalRelatedValList[] =
{
	{"END",				0x00000001},
	{"START",			0x00000002}
};

/* Rel type value */
static const ValueObj pCalReltypeValList[] =
{
	{"CHILD",			0x00000001},
	{"PARENT",			0x00000002},
	{"SIBLING",			0x00000004}
};

/* Value value */
static const ValueObj pCalValValList[] =
{
	{"BINARY",			0x00000001},
	{"BOOLEAN",			0x00000002},
	{"CAL-ADDRESS",		0x00000004},
	{"CID",				0x00000008},
	{"CONTENT-ID",		0x00000010},
	{"DATE",			0x00000020},
	{"DATE-TIME",		0x00000040},
	{"DURATION",		0x00000080},
	{"FLOAT",			0x00000100},
	{"INTEGER",			0x00000200},
	{"PERIOD",			0x00000400},
	{"PHONE-NUMBER",	0x00000800},
	{"RECUR",			0X00001000},
	{"TEXT",			0x00002000},
	{"TIME",			0x00004000},
	{"URI",				0x00008000},
	{"URL",				0x00010000},
	{"UTC-OFFSET",		0x00020000},
	{"VCALENDAR",		0x00040000},
	{"VEVENT",			0x00080000},
	{"VTODO",			0x00100000}
};

/* Type value */
static const ValueObj pCalTypeValList[] =
{
	{"AIFF",		0x00000001},
	{"BBS",			0x00000002},
	{"CAR",			0x00000004},
	{"CELL",		0x00000008},
	{"DOM",			0x00000010},
	{"FAX",			0x00000020},
	{"GIF",			0x00000040},
	{"HOME",		0x00000080},
	{"INTL",		0x00000100},
	{"INTERNET",	0x00000200},
	{"ISDN",		0x00000400},
	{"JPEG",		0x00000800},
	{"MODEM",		0x00001000},
	{"MSG",			0x00002000},
	{"PAGER",		0x00004000},
	{"PARCEL",		0x00008000},
	{"PCM",			0x00010000},
	{"PCS",			0x00020000},
	{"PNG",			0x00040000},
	{"POSTAL",		0x00080000},
	{"PREF",		0x00100000},
	{"VCARD",		0x00200000},
	{"VIDEO",		0x00400000},
	{"VOICE",		0x00800000},
	{"WAVE",		0x01000000},
	{"WBMP",		0x02000000},
	{"WORK",		0x04000000},
	{"X400",		0x08000000}
};

/* Expect value */
static const ValueObj pCalExpectValList[] =
{
	{"FYI",			0x00000001},
	{"IMMEDIATE",	0x00000002},
	{"REQUEST",		0x00000004},
	{"REQUIRE",		0x00000008}
};

/* Role value */
static const ValueObj pCalRoleValList[] =
{
	{"ATTENDEE",		0x00000001},
	{"CHAIR",			0x00000002},
	{"DELEGATE",		0x00000004},
	{"NON_PARTICIPANT",	0x00000008},
	{"OPT_PARTICIPANT",	0x00000010},
	{"ORGANIZER",		0x00000020},
	{"OWNER",			0x00000040},
	{"REQ_PARTICIPANT",	0x00000080}
};

/* RSVP value */
static const ValueObj pCalRSVPValList[] =
{
	{"false",		0x00000001},
	{"NO",			0x00000002},
	{"true",		0x00000004},
	{"YES",			0x00000008}
};

/* Status value */
static const ValueObj pCalStatusValList[] =
{
	{"ACCEPTED",	0x00000001},
	{"COMPLETED",	0x00000002},
	{"CONFIRMED",	0x00000004},
	{"DECLINED",	0x00000008},
	{"DELEGATED",	0x00000010},
	{"NEEDS ACTION",	0x00000020},
	{"SENT",		0x00000040},
	{"TENTATIVE",	0x00000080},
};

/****************************************************************************************************/
/*											FUNCTION DECLARATION									*/
/****************************************************************************************************/
static int __VCalGetName( char*, char**, int );
static int __VCalGetValue( char*, const ValueObj*, int );
static int __VCalGetTypeName( char*, int*, int *);
static int __VCalGetParamName( char*, int*, int *);
static char*	__VCalGetParamValue( char*, int*, int *);
static char*	__VCalGetTypeValue( char*, int*, int*, int, VObject* );
static char*	__VCalParamEncode( VObject*, int *);


/*
 * VIsVCalFile() verify VCalendar file.
 *
 * @param       pVCalRaw           Data which will be encoded
 * @return      int                 result (TRUE or FALSE)
 */
static int __VIsVCalFile(char *pVCalRaw)
{
	char *pszVCalBegin = "BEGIN:VCALENDAR";
	char *pszVCalEnd = "END:VCALENDAR";

	/*for(i=0; i<15; i++)
	  {
	  if(*pszVCalBegin++ != *pVCalRaw++)
	  return false;
	  }*/

	DBG("raw data:%s",pVCalRaw);
	if( strstr(pVCalRaw, pszVCalBegin) == NULL)
		return false;

	if( strstr(pVCalRaw, pszVCalEnd) == NULL)
		return false;
	return true;
}



/*
 * vCalFindName() compares the string and VCal type, parameter name.
 *
 * @param       string              Name which will compare
 * @param		list[]				Name list of VCal type and param
 * @param		size				Number of total element of list
 * @return      index               The index in the list
 */
static int __VCalGetName( char *szString, char** pszList, int size )
{
	int		high, low, i, diff;

	DBG( "__VCalGetName() enter..\n");


	for ( low = 0, high = size - 1; high >= low; diff < 0 ? ( low = i+1 ) : ( high = i-1 ) )
	{
		i = ( low + high ) / 2;
		diff = strcmp( pszList[i], szString );
		if ( diff == 0 )  	/* success: found it */
			return i;
	}
	return UNKNOWN_NAME;
}


/*
 * vCalFindValue() compares the string and VCal type, parameter value.
 *
 * @param       string              Value which will compare
 * @param		list[]				Value list of VCal type and param
 * @param		size				Number of total element of list
 * @return      index               The index in the list
 */
static int __VCalGetValue( char *szString, const ValueObj pList[], int size )
{
	int				high, low, i, diff;
	char*			szTemp = szString;
	unsigned int	k;

	DBG( "__VCalGetValue enter()..\n");

	for ( k = 0; k < strlen( szTemp ); k++ )
	{
		szTemp[k] = toupper( szTemp[k] );
	}

	for ( low = 0, high = size - 1; high >= low; diff < 0 ? ( low = i+1 ) : ( high = i-1 ) )
	{
		i = ( low + high ) / 2;
		diff = strcmp( pList[i].szName, szTemp );
		if ( diff == 0 )	/* success: found it */
			return pList[i].flag;
	}
	return UNKNOWN_NAME;
}


/*
 * vCalTypeName() fine the type name and returns the index number
 *
 * @param       pVCalRaw             The raw data
 * @param		status				Decoder status
 * @return      res              	The index in type list
 */
static int __VCalGetTypeName( char *pVCalRaw, int *pStatus, int *pDLen )
{
	char	name[MAX_CAL_TYPE_NAME];
	char	c = VTYPE_TOKEN_SEMICOLON;
	int	res=UNKNOWN_NAME;
	int	i;
	int	index = 0;

	DBG( "__VCalGetTypeName() enter \n");

	//	while ( true )
	while ( pVCalRaw!= NULL )
	{
		c = *pVCalRaw;

		pVCalRaw++;
		( *pDLen )++;

		if(index >= MAX_CAL_TYPE_NAME){
			DBG( "UNKNOWN_NAME!!(pVCalRaw(%s),%d)\n",pVCalRaw,index);
			res = UNKNOWN_NAME;
			if ( ( c == VTYPE_TOKEN_SEMICOLON ) || ( c == VTYPE_TOKEN_COLON ) )
				break;
			//return UNKNOWN_NAME;
		}
		else
		{
			if ( ( c == VTYPE_TOKEN_SEMICOLON ) || ( c == VTYPE_TOKEN_COLON ) )
			{
				name[index] = '\0';
				_VRLSpace( name );
				_VRTSpace( name );
				for ( i = 0; i < index; i++ )
					name[i] = toupper( name[i] );

				res = __VCalGetName( name, ( char** )pszCalTypeList, VCAL_TYPE_NUM );
				break;
			}
			else if ( ( c == '\r' ) || ( c == '\n' ) || ( _VIsSpace( c ) ) ) ;
			else
				name[index++] = c;
		}
	}

	if ( c == VTYPE_TOKEN_SEMICOLON )
		*pStatus = VCAL_PARAM_NAME_STATUS;
	else
		*pStatus = VCAL_TYPE_VALUE_STATUS;

	DBG( "###########- __VCalGetTypeName() end..\n");

	return res;
}


/*
 * vCalParamName() fine the param name and returns the index number
 *
 * @param       pVCalRaw             The raw data
 * @param		status				Decoder status
 * @return      res              	The index in param list
 */
static int __VCalGetParamName( char *pVCalRaw, int *pStatus, int *pDLen )
{
	char	name[50];
	char	c;
	int		res;
	int		i;
	int		index = 0;
	char*	pTemp = pVCalRaw;

	DBG( "__VCalGetParamName() enter..\n");

	//	while ( true )
	while ( pVCalRaw!= NULL )
	{
		c = *pVCalRaw;
		pVCalRaw++;
		( *pDLen )++;
		if ( c == VTYPE_TOKEN_EQUAL )
		{
			name[index] = '\0';
			_VRLSpace( name );
			_VRTSpace( name );
			for ( i = 0; i < index; i++ )
				name[i] = toupper( name[i] );

			res = __VCalGetName( name, ( char** )pszCalParamList, VCAL_PARAM_NUM );
			*pStatus = VCAL_PARAM_VALUE_STATUS;
			return res;
		}
		else if ( c == VTYPE_TOKEN_COLON )
		{
			*pStatus = VCAL_PARAM_VALUE_STATUS;
			pVCalRaw = pTemp;
			(*pDLen) = 0;
			return UNKNOWN_NAME;
		}
		else if ( c == VTYPE_TOKEN_SEMICOLON )
		{
			*pStatus = VCAL_PARAM_NAME_STATUS;
			pVCalRaw = pTemp;
			( *pDLen ) = 0;
			return UNKNOWN_NAME;
		}
		else if ( ( c == '\r' ) || ( c == '\n' ) || ( _VIsSpace( c ) ) ) ;
		else
			name[index++] = c;
	}
	return UNKNOWN_NAME;
}


/*
 * vCalParamValue() fine the param value and returns value.
 *
 * @param       pVCalRaw             The raw data
 * @param		status				Decoder status
 * @return      buffer              The result value
 */
static char* __VCalGetParamValue( char *pVCalRaw, int *pStatus, int *pDLen )
{
	char*	pBuf = NULL;
	char	c;
	int		len = 0;
	char*	pTemp = pVCalRaw;
	char	buf[100];
	char	delimiter;

	DBG( "__VCalGetParamValue() enter..\n");

	//	while ( true )
	while ( pVCalRaw!= NULL )
	{
		c = *pVCalRaw;
		pVCalRaw++;
		( *pDLen )++;
		if ( c == VTYPE_TOKEN_SEMICOLON )
		{
			buf[len] = '\0';
			if ( ( pBuf = ( char * )malloc( len+1 ) ) == NULL )
			{
				DBG( "__VCalGetParamValue():malloc Failed\n");
				return NULL;
			}
			//		memcpy( pBuf, pTemp, len-1 );
			strcpy( pBuf, buf );
			*( pBuf + len ) = '\0';
			_VRLSpace( pBuf );
			_VRTSpace( pBuf );
			*pStatus = VCAL_PARAM_NAME_STATUS;
			return pBuf;
		}
		else if ( c == VTYPE_TOKEN_COLON )
		{
			buf[len] = '\0';
			if ( ( pBuf = ( char * )malloc( len+1 ) ) == NULL )
			{
				DBG( "__VCalGetParamValue():malloc Failed\n");
				return NULL;
			}
			//		memcpy( pBuf, pTemp, len-1 );
			strcpy( pBuf, buf );
			*( pBuf + len ) = '\0';
			_VRLSpace( pBuf );
			_VRTSpace( pBuf );
			*pStatus = VCAL_TYPE_VALUE_STATUS;
			return pBuf;
		}
		else if ( c == VTYPE_TOKEN_COMMA )
		{
			buf[len] = '\0';
			if ( ( pBuf = ( char * )malloc( len+1 ) ) == NULL )
			{
				DBG( "__VCalGetParamValue():malloc Failed\n");
				return NULL;
			}
			//		memcpy( pBuf, pTemp, len-1 );
			strcpy( pBuf, buf );
			*( pBuf + len ) = '\0';
			_VRLSpace( pBuf );
			_VRTSpace( pBuf );
			*pStatus = VCAL_PARAM_VALUE_STATUS;
			return pBuf;
		}
		else if ( c == VTYPE_TOKEN_QUOTE || c == VTYPE_TOKEN_DBLQUOTE )
		{
			delimiter = c;
			pTemp = pVCalRaw;
			//			while ( true )
			while ( pVCalRaw != NULL )
			{
				c = *pVCalRaw;
				pVCalRaw++;
				( *pDLen )++;
				len++;
				if ( c == delimiter )
				{
					buf[len] = '\0';
					if ( ( pBuf = ( char * )malloc( len+1 ) ) == NULL )
					{
						DBG( "__VCalGetParamValue():malloc Failed\n");
						return NULL;
					}
					//	memcpy( pBuf, pTemp, len-1 );
					strcpy( pBuf, buf );
					*( pBuf + len ) = '\0';
					_VRLSpace( pBuf );
					_VRTSpace( pBuf );
					*pStatus = VCAL_PARAM_VALUE_STATUS;
					//while ( true )
					while ( pVCalRaw != NULL )
					{
						c = *pVCalRaw;
						pVCalRaw++;
						( *pDLen )++;
						len++;
						if ( c == VTYPE_TOKEN_COLON ) break;
					}
					return pBuf;
				}
				buf[len++] = c;
			}
		}
		else
			buf[len++] = c;
	}
	return pBuf;
}

int _gbase64Decode( char *Dest, char *Src )
{
	gsize len;

	guchar *buf = g_base64_decode(Src,&len);
	memcpy(Dest,buf,len);
	g_free(buf);
	DBG("_gbase64Decode before:\n%s \n",Src);
	DBG("_gbase64Decode after:\n%s \n",Dest);
	return len;
}

void _gbase64Encode( char *Dest, char *Src, int len )
{
	gchar *buf = g_base64_encode((guchar *)Src, strlen(Src));
	strcpy(Dest,buf);
	DBG("_gbase64Decode before:\n%s \n",Src);
	DBG("_gbase64Decode after:\n%s \n",Dest);
	g_free(buf);
}


/*
 * vCalTypeValue() fine the type value and returns value.
 *
 * @param       VCalRaw            The raw data
 * @param		status				Decoder status
 * @return      buffer              The result value
 */
static char* __VCalGetTypeValue( char *pVCalRaw, int *pStatus, int *pDLen, int enc, VObject* pType )
{
	char*	pBuf = NULL;
	char	c, c1, c2;
	int		len = 0;
	char*	pTemp = pVCalRaw;
	char*	pTmpBuf = NULL;
	int		bufferCount;
	int		num;
	bool	bEscape = false;

	DBG( "__VCalGetTypeValue() enter..\n");

	//	while ( true )
	while ( pVCalRaw!= NULL )
	{
		c = *pVCalRaw;
		pVCalRaw++;
		( *pDLen )++;
		len++;

		if ( c == VTYPE_TOKEN_SEMICOLON && bEscape == false )
		{
			if ( ( pBuf = ( char * )malloc( len ) ) == NULL )
			{
				DBG( "__VCalGetTypeValue():malloc Failed\n");
				return NULL;
			}
			memcpy( pBuf, pTemp, len-1 );
			*( pBuf + len -1 ) = '\0';
			_VRLSpace( pBuf );
			_VRTSpace( pBuf );
			_VUnescape( pBuf );
			*pStatus = VCAL_TYPE_VALUE_STATUS;
			if ( enc & pCalEncValList[1].flag )
			{
				bufferCount = ( len * 6 / 8 ) + 2;
				if ( ( pTmpBuf = ( char * )malloc( bufferCount ) ) == NULL )
				{
					DBG( "__VCalGetTypeValue():malloc Failed\n");
					free(pBuf);

					return NULL;
				}
				memset( pTmpBuf, '\0', bufferCount );

				num = _gbase64Decode( pTmpBuf, pBuf );

				if ( pType != NULL )
					pType->numOfBiData = num;
				free( pBuf );
				return pTmpBuf;
			}
			if ( enc & pCalEncValList[2].flag )
			{
				int i = 0, j = 0;

				while ( pBuf[i] )
				{
					if ( pBuf[i] == '\n' || pBuf[i] == '\r' )
					{
						i++;
						if ( pBuf[i] == '\n' || pBuf[i] == '\r' )
							i++;

						if ( pBuf[j-1] == '=' ) j--;
					}
					else
					{
						pBuf[j++] = pBuf[i++];
					}
				}
				pBuf[j] = '\0';

				_VQPDecode( pBuf );
				_VRLSpace( pBuf );
				_VRTSpace( pBuf );
			}
			return pBuf;
		}
		else if ( c == VTYPE_TOKEN_SEMICOLON && bEscape == true )
		{
			bEscape = false;
		}
		else if ( c == '\\' )
		{
			bEscape = true;
		}
		else if ( bEscape == true && c != VTYPE_TOKEN_SEMICOLON )
		{
			bEscape = false;
		}
		else if ( ( c == '\r' ) || ( c == '\n' ) )
		{
			c2 = *( pVCalRaw-2 );

			if ( c2 == '=' && ( enc & pCalEncValList[2].flag ) )
			{
				c1 = *pVCalRaw;
				if ( ( c1 == '\r' ) || ( c1 == '\n' ) )
				{
					pVCalRaw += 1;
					( *pDLen ) += 1; len++;
				}
			}
			else
			{
				if ( ( pBuf = ( char * )malloc( len ) ) == NULL )
				{
					DBG( "__VCalGetTypeValue():malloc Failed\n");
					return NULL;
				}

				memcpy( pBuf, pTemp, len-1 );

				*( pBuf + len -1 ) = '\0';
				_VRLSpace( pBuf );
				_VRTSpace( pBuf );
				_VUnescape( pBuf );
				*pStatus = VCAL_TYPE_NAME_STATUS;

				c1 = *( pVCalRaw );
				if ( ( c1 == '\r' ) || ( c1 == '\n' ) )
				{
					pVCalRaw += 1;
					( *pDLen ) += 1;
				}

				if ( enc & pCalEncValList[1].flag )
				{

					bufferCount = ( len * 6 / 8 ) + 2;
					if ( ( pTmpBuf = ( char * )malloc( bufferCount ) ) == NULL )
					{
						DBG( "__VCalGetTypeValue():malloc Failed\n");
						free(pBuf);
						return NULL;
					}
					memset( pTmpBuf, '\0', bufferCount );
					len = _gbase64Decode( pTmpBuf, pBuf );
					if ( pType != NULL )
						pType->numOfBiData = len;
					free( pBuf );
					return pTmpBuf;
				}
				if ( enc & pCalEncValList[2].flag )
				{
					int i = 0, j = 0;

					while ( pBuf[i] )
					{
						if ( pBuf[i] == '\n' || pBuf[i] == '\r' )
						{
							i++;
							if ( pBuf[i] == '\n' || pBuf[i] == '\r' )
								i++;

							if ( pBuf[j-1] == '=' ) j--;
						}
						else
						{
							pBuf[j++] = pBuf[i++];
						}
					}
					pBuf[j] = '\0';

					_VQPDecode( pBuf );
					_VRLSpace( pBuf );
					_VRTSpace( pBuf );
				}
				return pBuf;
			}

		}
		else if(enc == 0 && c == 0)  // VCAlENDAR type value  디코딩시  memory overlap  되는 현상 방지하기 위한 코드.
		{
			if ( ( pBuf = ( char * )malloc( len ) ) == NULL )
			{
				DBG( "__VCalGetTypeValue():malloc Failed\n");
				return NULL;
			}
			memcpy( pBuf, pTemp, len-1 );
			*( pBuf + len -1 ) = '\0';
			_VQPDecode( pBuf );
			_VRLSpace( pBuf );
			_VRTSpace( pBuf );
			return pBuf;
		}
	}
	DBG( "__VCalGetTypeValue() end..\n");

	return pBuf;
}


/*
 * vcal_decode() decode the VCal data and returns vObject struct
 *
 * @param       pVCalRaw             The raw data
 * @return      vObject             The result value
 */
	VTree*
vcal_decode( char *pVCalRaw )
{
	char*		szValue = NULL;
	char		c;
	VParam*		pTmpParam = NULL;
	VTree*		pRes = NULL;
	VTree*		pVCal = NULL;
	VObject*	pTemp = NULL;
	VTree*		pTmpTree = NULL;
	char*		szCalBegin = NULL;
	char*		szCalEnd = NULL;
	int			type, param;
	int			status = VCAL_TYPE_NAME_STATUS;
	int			done = false;
	int			valueCount = 0;
	int			len;
	int			dLen = 0;
	int			param_status = false;
	int			numberedParam = 0;
	int			enc = 0;
	unsigned int i;
	int			diff;

	DBG( "------------------------------------------enter vcal_decode()--------------------------\n");

	//SysRequireEx( pVCalRaw != NULL, NULL );
	retvm_if(NULL == pVCalRaw,NULL , "[ERROR]vcal_decode:Invalid parameter(pVCalRaw)!\n");

	_VManyCRLF2CRLF(pVCalRaw);

	if(__VIsVCalFile(pVCalRaw) == false)	// verify Vcalendar file
	{
		DBG( "not vcalendar file\n");
		return NULL;
	}

	//len = _VUnfolding( pVCalRaw );
	len = _VUnfoldingNoSpec(pVCalRaw, VCALENDAR);
	len = _VManySpace2Space( pVCalRaw );

	if ( len < 10 )
	{
		DBG("length is too short!!\n");
		return NULL;
	}

	//	while ( true )
	while ( pVCalRaw != NULL )
	{

		c = *pVCalRaw;
		if ( ( c == '\0' ) || done )
			break;

		switch ( status )
		{
		case VCAL_TYPE_NAME_STATUS:
			enc = 0;
			dLen = 0;
			numberedParam = 0;
			type = __VCalGetTypeName( pVCalRaw, &status, &dLen );
			pVCalRaw += dLen;

			switch ( type )
			{
			case VCAL_TYPE_BEGIN:
				DBG("VCAL_TYPE_BEGIN\n");
				dLen = 0;
				pVCal = ( VTree* )malloc( sizeof( VTree ) );

				if ( pVCal == NULL )
				{
					DBG( "vcal_decode():malloc Failed\n");
					return NULL;
				}
				pVCal->pTop = NULL;
				pVCal->pCur = NULL;
				pVCal->pNext = NULL;

				szCalBegin = __VCalGetTypeValue( pVCalRaw, &status, &dLen, 0, NULL );

				pVCalRaw += dLen;

				for ( i = 0; i < strlen( szCalBegin ); i++ )
					szCalBegin[i] = toupper( szCalBegin[i] );

				if ( ( diff = strcmp( "VCALENDAR", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = VCALENDAR;
					pRes = pVCal;
					pTmpTree = pRes;
				}
				else if ( ( diff = strcmp( "VEVENT", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = VEVENT;
					if(pTmpTree)
						pTmpTree->pNext = pVCal;
					pTmpTree = pVCal;
				}
				else if ( ( diff = strcmp( "VTODO", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = VTODO;
					pTmpTree->pNext = pVCal;
					pTmpTree = pVCal;
				}
				else if ( ( diff = strcmp( "VFREEBUSH", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = VFREEBUSY;
					pTmpTree->pNext = pVCal;
					pTmpTree = pVCal;
				}
				else if ( ( diff = strcmp( "VALARM", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = VALARM;
					pTmpTree->pNext = pVCal;
					pTmpTree = pVCal;
				}
				else if ( ( diff = strcmp( "VTIMEZONE", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = VTIMEZONE;
					pTmpTree->pNext = pVCal;
					pTmpTree = pVCal;
				}
				else if ( ( diff = strcmp( "VJOURNAL", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = VJOURNAL;
					pTmpTree->pNext = pVCal;
					pTmpTree = pVCal;
				}
				else if ( ( diff = strcmp( "STANDARD", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = STANDARD;
					pTmpTree->pNext = pVCal;
					pTmpTree = pVCal;
				}
				else if ( ( diff = strcmp( "DAYLIGHT", szCalBegin ) ) == 0 )
				{
					pVCal->treeType = DAYLIGHT;
					pTmpTree->pNext = pVCal;
					pTmpTree = pVCal;
				}

				free( szCalBegin );
				break;
			case VCAL_TYPE_END:
				dLen = 0;
				DBG("VCAL_TYPE_END\n");
				szCalEnd = __VCalGetTypeValue( pVCalRaw, &status, &dLen, 0, NULL );

				pVCalRaw += dLen;

				for ( i = 0; i < strlen( szCalEnd ); i++ )
					szCalEnd[i] = toupper(szCalEnd[i]);

				if ( ( diff = strcmp( "VCALENDAR", szCalEnd ) ) == 0 )
					done = true;
				free( szCalEnd );

				break;
				//case UNKNOWN_NAME:
				//	DBG(  "Unknown name!!\n");
				//	break;
			default:

				if ( pTmpTree == NULL )
				{
					ERR( "vcal_decode():pTmpTree is NULL\n");
					vcal_free_vtree_memory(pVCal);
					return NULL;

				}

				if ( ( pTemp = ( VObject* )malloc( sizeof( VObject ) ) ) == NULL )
				{
					ERR( "vcal_decode():malloc Failed\n");
					vcal_free_vtree_memory(pVCal);
					return NULL;
				}

				memset( pTemp, 0, sizeof( VObject ) );
				pTemp->property = type;

				if ( pTmpTree->pTop == NULL )
				{
					pTmpTree->pTop = pTemp;
					pTmpTree->pCur = pTemp;
				}
				else
				{

					pTmpTree->pCur->pSibling = pTemp;
					pTmpTree->pCur = pTemp;
				}

				break;
			}

			numberedParam = 0;
			param_status = false;
			valueCount = 0;
			break;
		case VCAL_PARAM_NAME_STATUS:
			dLen = 0;
			param = __VCalGetParamName( pVCalRaw, &status, &dLen );
			pVCalRaw += dLen;

			if ( param_status != true )
			{

				if ( pTmpTree == NULL )
				{
					return pVCal;
				}

				if ( ( pTmpTree->pCur->pParam = ( VParam* )malloc( sizeof( VParam ) ) ) == NULL )
				{
					DBG( "vcal_decode():malloc Failed\n");
					//SysSetLastError( VDATA_ERROR_MEMALLOC_FAILED );
					return pVCal;
				}
				param_status = true;
				pTmpParam = pTmpTree->pCur->pParam;
				memset( pTmpParam, 0, sizeof( VParam ) );
			}
			else
			{
				if ( pTmpParam == NULL )
				{
					return pVCal;
				}

				if ( ( pTmpParam->pNext = ( VParam* )malloc( sizeof( VParam ) ) ) == NULL )
				{
					DBG( "vcal_decode():malloc Failed\n");
					//SysSetLastError( VDATA_ERROR_MEMALLOC_FAILED );
					return pVCal;
				}
				pTmpParam = pTmpParam->pNext;
				memset( pTmpParam, 0, sizeof( VParam ) );
			}
			pTmpParam->parameter = param;

			// poiema Go through to the next case statement.

		case VCAL_PARAM_VALUE_STATUS:
			dLen = 0;
			numberedParam = 0;
			if(pTmpParam == NULL)
				break;
			switch( pTmpParam->parameter )
			{
			case VCAL_PARAM_TYPE:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalTypeValList, VCAL_TYPE_PARAM_NUM );
				break;
			case VCAL_PARAM_VALUE:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalValValList, VCAL_VALUE_PARAM_NUM );
				break;
			case VCAL_PARAM_ENCODING:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalEncValList, VCAL_ENCODE_PARAM_NUM );
				enc = numberedParam;
				break;
			case VCAL_PARAM_ROLE:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalRoleValList, VCAL_ROLE_PARAM_NUM );
				break;
			case VCAL_PARAM_RSVP:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalRSVPValList, VCAL_RSVP_PARAM_NUM );
				break;
			case VCAL_PARAM_EXPECT:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalExpectValList, VCAL_EXPECT_PARAM_NUM );
				break;
			case VCAL_PARAM_STATUS:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalStatusValList, VCAL_STATUS_PARAM_NUM );
				break;
			case VCAL_PARAM_CUTYPE:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalCutypeValList, VCAL_CUTYPE_PARAM_NUM );
				break;
			case VCAL_PARAM_FBTYPE:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalFbtypeValList, VCAL_FBTYPE_PARAM_NUM );
				break;
			case VCAL_PARAM_PARTSTAT:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalPartstatValList, VCAL_PARTSTAT_PARAM_NUM );
				break;
			case VCAL_PARAM_RANGE:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalRangeValList, VCAL_RANGE_PARAM_NUM );
				break;
			case VCAL_PARAM_RELATED:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalRelatedValList, VCAL_RELATED_PARAM_NUM );
				break;
			case VCAL_PARAM_RELTYPE:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam |= __VCalGetValue( szValue, pCalReltypeValList, VCAL_RELTYPE_PARAM_NUM );
				break;
			case VCAL_PARAM_CHARSET:
			case VCAL_PARAM_CONTEXT:
			case VCAL_PARAM_LANGUAGE:
			case VCAL_PARAM_ALTREP:
			case VCAL_PARAM_CN:
			case VCAL_PARAM_DELEGATED_FROM:
			case VCAL_PARAM_DELEGATED_TO:
			case VCAL_PARAM_DIR:
			case VCAL_PARAM_FMTYPE:
			case VCAL_PARAM_MEMBER:
			case VCAL_PARAM_SENT_BY:
			case VCAL_PARAM_TZID:
				{
					char * pParamValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
					if (pParamValue != NULL)
					{
						free(pParamValue);
						pParamValue = NULL;
					}
				}
				break;

			default:
				szValue = __VCalGetParamValue( pVCalRaw, &status, &dLen );
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalTypeValList, VCAL_TYPE_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_TYPE;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalValValList, VCAL_VALUE_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_VALUE;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalEncValList, VCAL_ENCODE_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_ENCODING;
					enc = numberedParam;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalRoleValList, VCAL_ROLE_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_ROLE;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalRSVPValList, VCAL_RSVP_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_RSVP;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalExpectValList, VCAL_EXPECT_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_EXPECT;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalStatusValList, VCAL_STATUS_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_STATUS;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalCutypeValList, VCAL_CUTYPE_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_CUTYPE;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalFbtypeValList, VCAL_FBTYPE_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_FBTYPE;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalPartstatValList, VCAL_PARTSTAT_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_PARTSTAT;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalRangeValList, VCAL_RANGE_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_RANGE;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalRelatedValList, VCAL_RELATED_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_RELATED;
					break;
				}
				numberedParam = 0;
				numberedParam |= __VCalGetValue( szValue, pCalReltypeValList, VCAL_RELTYPE_PARAM_NUM );
				if ( numberedParam != UNKNOWN_NAME )
				{
					pTmpParam->parameter = VCAL_PARAM_RELTYPE;
					break;
				}
				numberedParam = 0;

				char * pTypeValue = __VCalGetTypeValue( pVCalRaw, &status, &dLen, 0, pVCal->pCur );
				if (pTypeValue != NULL)
				{
					free(pTypeValue);
					pTypeValue = NULL;
				}

				break;
			}
			pTmpParam->paramValue = numberedParam;
			if (szValue != NULL)
			{
				free(szValue);
				szValue = NULL;
			}
			pVCalRaw += dLen;
			break;

		case VCAL_TYPE_VALUE_STATUS:
			{
				dLen = 0;
				char *temp = NULL;
				temp = __VCalGetTypeValue( pVCalRaw, &status, &dLen, enc, pTmpTree->pCur );

				if(valueCount < VDATA_VALUE_COUNT_MAX) {
					pTmpTree->pCur->pszValue[valueCount] = temp;
					valueCount++;
				}
				else
					free(temp);
			}


			pTmpTree->pCur->valueCount = valueCount;
			pVCalRaw += dLen;
			break;
		}
		//		pVCalRaw += dLen;
		//		break;
	}

	if (!done )
	{
		DBG("@@@---- vcal_decode() lack VCLENDAR END and exit\n");
		vcal_free_vtree_memory(pVCal);
		return NULL;

	}

	DBG( "------------------------------------------exit vcal_decode()--------------------------\n");

	return pRes;
}

/*
 * vcal_free_vtree_memory() frees memory used for decoding.
 *
 * @param       pTree            VTree structure to be freed.
 * @return      If succeeds, return true, else false.
 */
	bool
vcal_free_vtree_memory( VTree* pTree )
{

	VObject*		pCurObj = NULL;
	VObject*		pNextObj = NULL;
	VTree*			pCurTree = NULL;
	VTree*			pNextTree = NULL;

	DBG( "vcal_free_vtree_memory() entered.\n");
	//SysRequireEx( pTree != NULL, false );
	retvm_if(NULL == pTree,NULL , "[ERROR]vcal_free_vtree_memory:Invalid parameter(pTree)!\n");

	if ((pTree->treeType == VCALENDAR) ||
			((pTree->treeType >= VEVENT) &&
			 (pTree->treeType <= DAYLIGHT)))
	{
		//continue
		;
	}
	else
	{
		return false;
	}


	pCurTree = pTree;

	while ( pCurTree )
	{
		pNextTree = pCurTree->pNext;
		pCurObj = pCurTree->pTop;

		while ( pCurObj )
		{
			int count;
			int i;

			pNextObj = pCurObj->pSibling;

			count = pCurObj->valueCount;

			for ( i = 0; i < count; i++ )
			{
				if ( pCurObj->pszValue[i] )
				{
					free( pCurObj->pszValue[i] );
					pCurObj->pszValue[i] = NULL;
				}
			}

#ifdef VDATA_GROUPNAME_SUPPORTED
			if ( pCurObj->pszGroupName )
			{
				free( pCurObj->pszGroupName );
				pCurObj->pszGroupName = NULL;
			}
#endif // VDATA_GROUPNAME_SUPPORTED

			if ( pCurObj->pParam )
			{
				VParam* pCurParam = NULL;
				VParam* pNextParam = NULL;

				pCurParam = pCurObj->pParam;

				while ( pCurParam )
				{
					pNextParam = pCurParam->pNext;

					free( pCurParam );
					pCurParam = NULL;

					pCurParam = pNextParam;
				}
			}

			free( pCurObj );
			pCurObj = NULL;

			pCurObj = pNextObj;
		}

		free( pCurTree );
		pCurTree = NULL;

		pCurTree = pNextTree;
	}

	DBG( "\n---------------------------exit vcal_free_vtree_memory--------- -------------\n");

	return true;
}


/*
 * vCaLTypeEncoder() compares the string and VCal type, parameter value.
 *
 * @param		typeObj				Data which will be encoded
 * @param		type				Name of the type
 * @return      char *              Encoded result
 */
static char* __VCalTypeEncode( VObject* pTypeObj, const char *pType )
{
	int			len;
	char*		pTemp = NULL;
	char*		szTypeValue = NULL;
	int			i;
	int			enc = 0;
	char*		pEncode = NULL;
	char*		pRes = NULL;
	int			total = 0;
	int			biLen = 0;
	char*		tempszTypeValue = NULL;

	DBG( "__VCalTypeEncode() enter..\n");

	len = strlen( pType );
	biLen = pTypeObj->numOfBiData;

	if ( ( szTypeValue = ( char * )malloc( total += ( len+1+10 ) ) ) == NULL ) {
		DBG( "VCalTypeEncode():malloc failed\n");
		return NULL;
	}

	memset( szTypeValue, '\0', ( len+1+10 ) );
	memcpy( szTypeValue, pType, len );

	pTemp = __VCalParamEncode( pTypeObj, &enc );
	if ( pTemp != NULL )
	{
		len = strlen( pTemp );
		tempszTypeValue = szTypeValue;
		if ( ( szTypeValue = ( char * )realloc( szTypeValue, ( total += len + 10 ) ) ) == NULL ){
			DBG( "__VCalTypeEncode():realloc failed\n");
			if (pTemp != NULL) {
				free(pTemp);
				pTemp = NULL;
			}

			if(tempszTypeValue)	{
				free(tempszTypeValue);
				tempszTypeValue = NULL;
			}
			return NULL;
		}
		strcat( szTypeValue, pTemp );
		free( pTemp );
	}

	tempszTypeValue = szTypeValue;
	if ( ( szTypeValue = ( char * )realloc( szTypeValue, ( total += 2 + 10 ) ) ) == NULL ){

		DBG( "__VCalTypeEncode():realloc failed\n");
		if(tempszTypeValue) {
			free(tempszTypeValue);
			tempszTypeValue = NULL;
		}
		return NULL;
	}
	strcat( szTypeValue, ":\0" );

	len = 0;
	for ( i = 0; i < pTypeObj->valueCount; i++ ){
		if( (pTypeObj->pszValue[i]==NULL)) {
			free(szTypeValue);
			return NULL;
		}

		len += strlen( pTypeObj->pszValue[i] );
	}

	int buf_len = 0;
	char *buf = NULL;
	if ( !( pEncode = ( char * )calloc(1, (len+30)*2 + 31 )) ) {
		free( szTypeValue );
		DBG( "__VCalTypeEncode():malloc failed\n");
		return NULL;
	}
	for ( i = 0; i < pTypeObj->valueCount; i++ ) {
		int len_i = strlen(pTypeObj->pszValue[i]);
		if( buf_len < len_i ) {
			free(buf);
			buf_len = len_i;
			if( !(buf = (char*) calloc(1, buf_len*2 + 1 )) ) {
				free( szTypeValue );
				DBG( "__VCalTypeEncode():malloc failed\n");
				free( pEncode );
				return NULL;
			}
		} else {
			if(buf)
				bzero(buf, buf_len);
		}

		if(buf)
		{
			strncpy (buf, pTypeObj->pszValue[i], len_i);
			_VEscape( buf );

			if( i )	strcat( pEncode, ";");
			strcat( pEncode, buf );
		}
	}
	free( buf );

	strcat( pEncode, "\0\0" );
	//	_VEscape( pEncode );
	len = strlen( pEncode );

	if ( enc & pCalEncValList[2].flag )
	{

		//if ( ( pRes = ( char * )malloc( len+40 ) ) == NULL )
		//if ( ( pRes = ( char * )malloc( len+40+10 ) ) == NULL )
		// Task description에 enter가 들어갈 경우 memory 모자람. len * 4 + 30 -> len * 6 + 30으로 변경 2004.3.12
		if ( ( pRes = ( char * )malloc( len*6+30 ) ) == NULL )
		{
			DBG( "__VCalTypeEncode():malloc failed\n");

			free( pEncode );
			free( szTypeValue );

			return NULL;
		}
		_VQPEncode( pRes, pEncode );
		free( pEncode );
		pEncode = NULL;
	}
	else if ( enc & pCalEncValList[1].flag )
	{

		//if ( ( pRes = ( char * )malloc( ( len * 8 / 6 ) + 4 ) ) == NULL )
		if ( ( pRes = ( char * )malloc( ( len * 8 / 6 ) + 4 + 10 ) ) == NULL )
		{
			DBG( "__VCalTypeEncode():malloc failed\n");
			free( pEncode );
			free( szTypeValue );

			return NULL;
		}

		//memset( pRes, '\0', ( ( len * 8 / 6 ) + 4 ) );
		memset( pRes, '\0', ( ( len * 8 / 6 ) + 4 + 10) );
		_gbase64Encode( pRes, pEncode, biLen );

		free( pEncode );
		pEncode = NULL;
	}
	else
	{

		//if ( ( pRes = ( char * )malloc( len+2 ) ) == NULL )
		if ( ( pRes = ( char * )malloc( len+2 + 10) ) == NULL )
		{
			DBG( "__VCalTypeEncode():malloc failed\n");

			free( pEncode );
			free( szTypeValue );

			return NULL;
		}
		//memset( pRes, '\0', ( len + 2 ) );
		memset( pRes, '\0', ( len + 2 + 10 ) );
		memcpy( pRes, pEncode, len );
		free( pEncode );
		pEncode = NULL;
	}

	strcat( pRes, "\r\n" );  //\n\0 -> \r\n 수정  ewpark 10.14th

	len = strlen( pRes );
	//if ( ( szTypeValue = ( char * )realloc( szTypeValue, ( total += ( len+3 ) ) ) ) == NULL )
	tempszTypeValue = szTypeValue;
	if ( ( szTypeValue = ( char * )malloc( ( total += ( len*4 +3 + 10 ) ) ) ) == NULL )
	{
		DBG( "__VCalTypeEncode():realloc failed\n");
		if (pEncode != NULL)
		{
			free( pEncode );
			pEncode = NULL;
		}
		if (pRes != NULL)
		{
			free( pRes );
			pRes = NULL;
		}
		if(tempszTypeValue)
		{
			free(tempszTypeValue);
			tempszTypeValue = NULL;
		}

		return NULL;
	}

	memset(szTypeValue,0x00,total);
	strncpy(szTypeValue,tempszTypeValue,total);
	strcat( szTypeValue, pRes );
	_VRLSpace( szTypeValue );
	_VRTSpace( szTypeValue );

	free(tempszTypeValue);
	free( pRes );
	if ( strlen( szTypeValue ) >= 75 )
	{
		if ( ( pEncode = ( char * )malloc( sizeof( char ) * ( strlen( szTypeValue )*4 + ( strlen( szTypeValue ) / 75 ) * 2 + 10 + 10 ) ) ) == NULL )
		{
			DBG( "__VCalTypeEncode():malloc failed\n");
			free( szTypeValue );
			return NULL;
		}

		_VFolding( pEncode, szTypeValue );
		free( szTypeValue );

		return pEncode;
	}

	DBG( "__VCalTypeEncode() end..\n");

	return szTypeValue;
}


/*
 * vcal_encode() compares the string and VCal type, parameter value.
 *
 * @param       pVCalRaw            Data which will be encoded
 * @return      char *              Encoded result
 */
	char*
vcal_encode( VTree* pVCalRaw )
{
	char*		pCalRes = NULL;
	char*		pTemp = NULL;
	int			len;
	int			total = 0;
	VTree*		pTmpTree = NULL;
	VObject*	pTmpObj = NULL;

	DBG( "vcal_encode() enter..\n");
	//SysRequireEx( pVCalRaw != NULL, NULL );
	if(pVCalRaw == NULL)
		return NULL;

	if ((pVCalRaw->treeType == VCALENDAR) ||
			((pVCalRaw->treeType >= VEVENT) &&
			 (pVCalRaw->treeType <= DAYLIGHT)))
	{
		//continue
		;
	}
	else
	{
		return NULL;
	}

	if ( ( pCalRes = ( char * )malloc( total += 2 ) ) == NULL )
	{
		DBG( "vcal_encode():malloc failed\n");
		return NULL;
	}
	memset( pCalRes, '\0', 1);

	pTmpTree = pVCalRaw;
	pTmpObj = pTmpTree->pTop;

	/*
		if( (pTmpObj == NULL) || (pTmpObj->property < 0) || (pTmpObj->valueCount < 0) )
		{
		if(pTmpObj !=NULL)
		DBG(  "pTmpObj = %d, pTmpObj->property = %d,pTmpObj->valueCount=%d \n",pTmpObj,pTmpObj->property,pTmpObj->valueCount);
		else
		DBG("pTmpObj is NULL");
		return NULL;
		}*/

	while ( true )
	{
		switch ( pTmpTree->treeType )
		{
		case VCALENDAR:

			// wyj add PRODID field,set PRODID length is 100
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 18 + 100+15) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:VCALENDAR\r\n" );
			strcat( pCalRes, "PRODID:-//Tizen //Calendar //EN\r\n" );

			strcat( pCalRes, "VERSION:1.0\r\n" );
			break;
		case VEVENT:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 15 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:VEVENT\r\n" );
			break;
		case VTODO:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 14 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:VTODO\r\n" );
			break;
		case VJOURNAL:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 17 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:VJOURNAL\r\n" );
			break;
		case VFREEBUSY:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 18 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:VFREEBUSY\r\n" );
			break;
		case VTIMEZONE:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 19 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:VTIMEZONE\r\n" );
			break;
		case VALARM:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 15 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:VALARM\r\n" );
			break;
		case STANDARD:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 17 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:STANDARD\r\n" );
			break;
		case DAYLIGHT:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 17 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "BEGIN:DAYLIGHT\r\n" );
			break;
		}

		while ( true )
		{
			if ( pTmpObj != NULL )
			{
				if( pTmpObj->property <0 || pTmpObj->property >= VCAL_TYPE_NUM )
				{
					if(pCalRes)
						free(pCalRes);
					return NULL;
				}
				if ( ( pTemp = __VCalTypeEncode( pTmpObj, pszCalTypeList[pTmpObj->property] ) ) != NULL )
				{
					len = strlen( pTemp );
					if ( ( pCalRes = ( char *)realloc( pCalRes, ( total += ( len + 10 ) ) ) ) == NULL )
					{
						DBG( "vcal_encode():realloc failed\n");
						free( pTemp );
						pTemp = NULL;
						return NULL;
					}
					strcat( pCalRes, pTemp );
					free( pTemp );
				}

				if ( pTmpObj->pSibling != NULL )
					pTmpObj = pTmpObj->pSibling;
				else
					break;
			}
			else
				break;
		}

		switch ( pTmpTree->treeType )
		{
			/*	case VCALENDAR:
				if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 15 ) ) ) == NULL )
				{
				DBG(  "vcal_encode():realloc failed\n");
				return NULL;
				}
				memcpy( pCalRes, "END:VCALENDAR\r\n", 17 );
				break; */
		case VEVENT:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 13 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "END:VEVENT\r\n" );
			break;
		case VTODO:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 12 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "END:VTODO\r\n" );
			break;
		case VJOURNAL:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 15 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "END:VJOURNALO\r\n" );
			break;
		case VFREEBUSY:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 16 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "END:VFREEBUSY\r\n" );
			break;
		case VTIMEZONE:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 16 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "END:VTIMEZONE\r\n" );
			break;
		case VALARM:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 13 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "END:VALARM\r\n" );
			break;
		case STANDARD:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 15 ) ) ) == NULL )
			{

				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "END:STANDARD\r\n" );
			break;
		case DAYLIGHT:
			if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 15 ) ) ) == NULL )
			{
				DBG( "vcal_encode():realloc failed\n");
				return NULL;
			}
			strcat( pCalRes, "END:DAYLIGHT\r\n" );
			break;
		}

		if ( pTmpTree->pNext != NULL )
			pTmpTree = pTmpTree->pNext;
		else
			break;
		pTmpObj = pTmpTree->pTop;
	}

	if ( ( pCalRes = ( char * )realloc( pCalRes, ( total += 16 ) ) ) == NULL )
	{

		DBG( "vcal_encode():realloc failed\n");
		return NULL;
	}
	strcat( pCalRes, "END:VCALENDAR\r\n" );  //\n\0 -> \r\n 수정  ewpark 10.14th

	DBG( "vcal_encode() end..\n");

	return pCalRes;
}



/*
 * vCalParamEncoder() compares the string and VCal type, parameter value.
 *
 * @param		typeObj				Data which will be encoded
 * @param		type				Name of the type
 */
static char* __VCalParamEncode( VObject* pTypeObj, int *pEnc )
{
	char *szParam = NULL;
	VParam *pTemp = NULL;
	int i;
	const ValueObj *pList;
	bool bSupported;
	int sNum;
	int shift;
	int len = 0;

	DBG( "__VCalParamEncode() enter..\n");
	retvm_if(NULL == pTypeObj,NULL , "[ERROR]__VCalParamEncode:Invalid parameter(pTypeObj)!\n");

	pTemp = pTypeObj->pParam;
	if ( pTemp != NULL )
	{
		if ( ( szParam = ( char *)malloc( len += 2 ) ) == NULL )
		{
			DBG( "__VCalParamEncode():malloc failed\n");
			return NULL;
		}
		memcpy( szParam, "\0\0", 2 );
	}

	while ( pTemp != NULL )
	{
		bSupported = false;

		if ( ( szParam = ( char * )realloc( szParam, ( len += 15 ) ) ) == NULL )
		{

			DBG( "__VCalParamEncode():realloc failed\n");
			return NULL;
		}

		strcat( szParam, ";" );
		strcat( szParam, pszCalParamList[pTemp->parameter] );
		strcat( szParam, "=" );

		switch ( pTemp->parameter )
		{
		case VCAL_PARAM_ENCODING:
			*pEnc = pTemp->paramValue;
			shift = VCAL_ENCODE_PARAM_NUM;
			pList = pCalEncValList; bSupported = true;
			break;
		case VCAL_PARAM_CHARSET:
			shift = VCAL_CHARSET_PARAM_NUM;
			pList = pCalCharsetValList; bSupported = true;
			break;
		case VCAL_PARAM_TYPE:
			shift = VCAL_TYPE_PARAM_NUM;
			pList = pCalTypeValList; bSupported = true;
			break;
		case VCAL_PARAM_VALUE:
			shift = VCAL_VALUE_PARAM_NUM;
			pList = pCalValValList; bSupported = true;
			break;
		case VCAL_PARAM_EXPECT:
			shift = VCAL_EXPECT_PARAM_NUM;
			pList = pCalExpectValList; bSupported = true;
			break;
		case VCAL_PARAM_ROLE:
			shift = VCAL_ROLE_PARAM_NUM;
			pList = pCalRoleValList; bSupported = true;
			break;
		case VCAL_PARAM_RSVP:
			shift = VCAL_RSVP_PARAM_NUM;
			pList = pCalRSVPValList; bSupported = true;
			break;
		case VCAL_PARAM_STATUS:
			shift = VCAL_STATUS_PARAM_NUM;
			pList = pCalStatusValList; bSupported = true;
			break;
		case VCAL_PARAM_CUTYPE:
			shift = VCAL_CUTYPE_PARAM_NUM;
			pList = pCalCutypeValList; bSupported = true;
			break;
		case VCAL_PARAM_FBTYPE:
			shift = VCAL_FBTYPE_PARAM_NUM;
			pList = pCalFbtypeValList; bSupported = true;
			break;
		case VCAL_PARAM_PARTSTAT:
			shift = VCAL_PARTSTAT_PARAM_NUM;
			pList = pCalPartstatValList; bSupported = true;
			break;
		case VCAL_PARAM_RANGE:
			shift = VCAL_RANGE_PARAM_NUM;
			pList = pCalRangeValList; bSupported = true;
			break;
		case VCAL_PARAM_RELATED:
			shift = VCAL_RELATED_PARAM_NUM;
			pList = pCalRelatedValList; bSupported = true;
			break;
		case VCAL_PARAM_RELTYPE:
			shift = VCAL_RELTYPE_PARAM_NUM;
			pList = pCalReltypeValList; bSupported = true;
			break;
		default:

			if ( ( szParam = ( char *)realloc( szParam, 5 ) ) == NULL )
			{
				DBG( "__VCalParamEncode():realloc failed\n");
				return NULL;
			}
			strcat( szParam, "NONE" );
			break;
		}

		if ( bSupported == true )
		{
			for ( i = 0, sNum = 0x00000001; i < shift; i++ )
			{
				if ( pTemp->paramValue & sNum )
				{
					if ( ( szParam = ( char * )realloc( szParam,
									( len += ( strlen( pList[i].szName ) + 2 ) ) ) ) == NULL )
					{

						DBG( "__VCalParamEncode():realloc failed\n");
						return NULL;
					}
					strcat( szParam, pList[i].szName );
					strcat( szParam, ", \0" );
				}
				sNum <<= 1;
			}
		}

		for ( i = strlen( szParam ); i > 0 ; i-- )
		{
			if ( szParam[i] == ' ' && szParam[i-1] == ',' )
			{
				szParam[i-1] = '\0';
				break;
			}
		}

		if ( pTemp->pNext != NULL )
			pTemp = pTemp->pNext;
		else
			break;
	}

	DBG( "\n ------end __VCalParamEncode ---------..\n");

	return szParam;
}

bool __cal_vcalendar_get_week_day_string(struct tm *start_time, char *s2)
{
	char *date_string[7] = { "SU","MO","TU","WE","TH","FR","SA" };

	struct tm temp;
	time_t t = timegm(start_time);
	gmtime_r(&t, &temp);

	if(temp.tm_wday < 0 || temp.tm_wday > 6 )
	{
		return false;
	}
	else
	{
		snprintf(s2, 3, date_string[temp.tm_wday]);
	}

	return true;
}

// in calendar schedule and todo data structure, the remind_tick is always 1
	bool
__cal_vcalendar_parse_reminder_time(const char * str, cal_sch_category_t category, struct tm * event_start_time, int * remind_tick, cal_sch_remind_tick_unit_t * remind_tick_unit)
{
	assert(str != NULL);

	struct tm 	tm={0,};
	struct tm  base_tm={0,};

	cal_vcalendar_convert_utc_str_to_tm(str, &tm);
	base_tm.tm_year = event_start_time->tm_year;
	base_tm.tm_mon = event_start_time->tm_mon;
	base_tm.tm_mday = event_start_time->tm_mday;
	if ((category == CAL_SCH_SPECIAL_OCCASION) || (category == CAL_SCH_BIRTHDAY))
	{
		base_tm.tm_min = tm.tm_min;
		base_tm.tm_hour = tm.tm_hour;
	}
	else
	{
		base_tm.tm_min = event_start_time->tm_min;
		base_tm.tm_hour = event_start_time->tm_hour;
	}
	base_tm.tm_sec = 0;
	tm.tm_sec = 0;
	time_t gap_time = timegm(&base_tm) - timegm(&tm);

	*remind_tick = 1;

	if (gap_time < 0)
	{
		gap_time = 0 - gap_time;
		*remind_tick_unit = CAL_SCH_TIME_UNIT_MIN;
		*remind_tick = 0;
	}
	else if ((gap_time >= 0) && (gap_time < 60*60))
	{
		*remind_tick_unit = CAL_SCH_TIME_UNIT_MIN;
		*remind_tick = gap_time/60;
	}
	else if ((gap_time >= 60*60) && (gap_time < 60*60*24))
	{
		*remind_tick_unit = CAL_SCH_TIME_UNIT_HOUR;
		*remind_tick = gap_time/(60*60);

	}
	else if ((gap_time >= 60*60*24) && (gap_time < 60*60*24*7))
	{
		*remind_tick_unit = CAL_SCH_TIME_UNIT_DAY;
		*remind_tick = gap_time/(60*60*24);

	}
	else if ((gap_time >= 60*60*24*7) && (gap_time < 60*60*24*30))
	{
		*remind_tick_unit = CAL_SCH_TIME_UNIT_WEEK;
		*remind_tick = gap_time/(60*60*24*7);

	}
	else if ((gap_time >= 60*60*24*30) && (gap_time < 60*60*24*365))
	{
		*remind_tick_unit = CAL_SCH_TIME_UNIT_MONTH;
		*remind_tick = gap_time/(60*60*24*30);

	}

	return true;
}

// string format: D1 20020305T235959Z
	bool
__cal_vcalendar_parse_sch_repeat_rule(const char *szText, cal_repeat_term_t * repeat_term, int * repeatInterval, struct tm * repeat_end_date,int *repeat_count)
{
	char	szBuff[4];
	int		startPos, strLen;
	int		i, j;
	char	c, c1, c2;
	char 	*date_pos = NULL;

	startPos = 1;
	j = 0;

	//check repeat term rule
	if ( szText[0] == 'D' )
	{
		*repeat_term = CAL_REPEAT_EVERY_DAY;
		startPos = 1;
	}
	else if ( szText[0] == 'W' )
	{
		*repeat_term = CAL_REPEAT_EVERY_WEEK;
		startPos = 1;
	}
	else if ( szText[0] == 'M' )
	{
		*repeat_term = CAL_REPEAT_EVERY_MONTH;
		if ( szText[1] == 'P' )
		{
			startPos = 2;
		}
		else if ( szText[1] == 'D' )
		{
			startPos = 2;
		}
	}
	else if ( szText[0] == 'Y' )
	{
		*repeat_term = CAL_REPEAT_EVERY_YEAR;
		if ( szText[1] == 'M' )
		{
			startPos = 2;
		}
		else if ( szText[1] == 'D' )
		{
			startPos = 2;
		}
	}
	//check repeat interval
	strLen = strlen(szText);
	for ( i = startPos ; i < strLen ; i++ )
	{
		if ( j < 4 )
		{
			if (szText[i] != ' ')
			{
				szBuff[j] = szText[i];
				j++;
			}
			else
			{
				break;
			}
		}
		else
		{
			return false;
		}

	}

	szBuff[j] = '\0';
	*repeatInterval = atol(szBuff);

	startPos = i + 1;

	date_pos = strchr(szText + startPos, 'T');

	if(date_pos == NULL)
	{
		date_pos = strchr(szText + startPos, '#');
		*repeat_count = atol(date_pos+1);
	}
	else
	{
		while(date_pos != NULL) {
			c1 = *(date_pos - 1);
			c2 = *(date_pos + 1);

			if(isdigit(c1) && isdigit(c2))
			{
				do
				{
					c=*(--date_pos);
				}while(c!=' ');
				date_pos = date_pos + 1; // Skip ' ';
				cal_vcalendar_convert_utc_str_to_tm(date_pos, repeat_end_date);
				break;
			}
			if(date_pos + 1 == NULL)
			{
				break;
			}

			date_pos = strchr(date_pos+1, 'T');
		}
	}

	return true;
}

	bool
__cal_vcalendar_parse_sch_repeat_rule_for_ical(VObject* object , cal_repeat_term_t * repeat_term, int * repeatInterval, struct tm * repeat_end_date)
{
	int i = 0;
	int j = 0;
	int m = 0;
	int value_count = 0;
	char	szBuff[4];
	int startPos = 0;
	int strLen = 0;

	if (strcmp(object->pszValue[0], "FREQ=DAILY") == 0)
	{
		*repeat_term = CAL_REPEAT_EVERY_DAY;
	}
	else if (strcmp(object->pszValue[0], "FREQ=WEEKLY") == 0)
	{
		*repeat_term = CAL_REPEAT_EVERY_WEEK;
	}
	else if (strcmp(object->pszValue[0], "FREQ=MONTHLY") == 0)
	{
		*repeat_term = CAL_REPEAT_EVERY_MONTH;
	}
	else if (strcmp(object->pszValue[0], "FREQ=YEARLY") == 0)
	{
		*repeat_term = CAL_REPEAT_EVERY_YEAR;
	}
	else
	{
		*repeat_term = CAL_REPEAT_NONE;
	}

	*repeatInterval = 1;
	value_count = object->valueCount;
	for (i = 1; i < value_count; i ++)
	{
		if (strstr(object->pszValue[i], "INTERVAL=") != NULL)
		{
			startPos = 9;
			//check repeat interval
			strLen = strlen(object->pszValue[i]);
			for ( m = startPos ; m < strLen ; m++ )
			{
				if ( j < 4 )
				{
					if (object->pszValue[i][m] != ' ')
					{
						szBuff[j] = object->pszValue[i][m];
						j++;
					}
					else
					{
						break;
					}
				}
				else
				{
					return false;
				}

			}
			szBuff[j] = '\0';
			*repeatInterval = atol(szBuff);
		}
		else if (strstr(object->pszValue[i], "UNTIL=") != NULL)
		{
			startPos = 6;
			if (object->pszValue[i] + startPos != NULL)
			{
				cal_vcalendar_convert_utc_str_to_tm(object->pszValue[i] + startPos, repeat_end_date);
			}
		}
		else
		{
			;
		}
	}

	CALS_DBG( "\n-------------------__cal_vcalendar_parse_sch_repeat_rule_for_ical repeat item is %d, interval is %d --------------------\n",*repeat_term,*repeatInterval);

	return true;
}

	struct tm *
__cal_vcalendar_compute_pre_time(struct tm* start_tm,
	struct tm *alarm_tm, const int remind_tick, const cal_sch_remind_tick_unit_t unit,struct tm *ret_tm)
{
	start_tm->tm_sec = 0;
	alarm_tm->tm_sec = 0;
	//	time_t gap_time = cals_mktime(alarm_tm) - cals_mktime(start_tm);
	time_t 	timep = timegm(alarm_tm);
	time_t 	t = 0;

	// Fix for prevent - B.
	switch (unit )
	{
	case CAL_SCH_TIME_UNIT_MIN:
		t = timep - 60*remind_tick;
		break;

	case CAL_SCH_TIME_UNIT_HOUR:
		t = timep - 60*60*remind_tick;
		break;

	case CAL_SCH_TIME_UNIT_DAY:
		t = timep - 24*60*60*remind_tick;
		break;
	case CAL_SCH_TIME_UNIT_WEEK:
		t = timep - 7*24*60*60*remind_tick;
		break;

	case CAL_SCH_TIME_UNIT_MONTH:
		t = timep - 30*24*60*60*remind_tick;
		break;

	default:
		break;

	}

	return gmtime_r(&t,ret_tm);
}


	bool
__cal_vcalendar_sch_vtree_add_object(VTree *tree, const cal_sch_full_t *sch, vCalType obj_type)
{
#if 1

	VObject*		object;
	int str_len = 0;

	assert((tree != NULL) && (sch != NULL));

	object = (VObject*)malloc(sizeof(VObject));
	if ( !object )
	{
		vcal_free_vtree_memory( tree );
		return false;
	}

	memset( object, 0, sizeof(VObject));

	switch(obj_type)
	{
	case VCAL_TYPE_CATEGORIES:
		{
			if(sch->cal_type == CAL_EVENT_TODO_TYPE)
			{
				break;
			}

			object->property = VCAL_TYPE_CATEGORIES;

			object->pszValue[0] = (char*)malloc(20);
			if ( !(object->pszValue[0]) )
			{
				vcal_free_vtree_memory( tree );
				CAL_FREE(object);

				return false;
			}

			memset(object->pszValue[0], 0, 20);

			switch (sch->sch_category)
			{
			case CAL_SCH_APPOINTMENT:
				strcpy( object->pszValue[0], "APPOINTMENT");
				break;

			case CAL_SCH_HOLIDAY:
				strcpy( object->pszValue[0], "HOLIDAY");
				break;

			case CAL_SCH_BUSSINESS:
				strcpy( object->pszValue[0], "BUSINESS");
				break;

			case CAL_SCH_SPECIAL_OCCASION:
				strcpy( object->pszValue[0], "SPECIAL OCCASION");
				break;

			case CAL_SCH_IMPORTANT:
				strcpy( object->pszValue[0], "IMPORTANT");
				break;

			case CAL_SCH_BIRTHDAY:
				strcpy( object->pszValue[0], "BIRTHDAY");
				break;

			default:
				strcpy( object->pszValue[0], "MISCELLANEOUS");
				break;

			}
			object->valueCount = 1;
		}
		break;

	case VCAL_TYPE_SUMMARY:
		{
			if (NULL != sch->summary && strlen(sch->summary) != 0)
			{
				object->property = VCAL_TYPE_SUMMARY;
				str_len = strlen(sch->summary);
				//str_len = (str_len>500)?500:str_len;
				CALS_DBG("-------------------__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_SUMMARY(%d) -------------",str_len);
				object->pParam = (VParam*)malloc(sizeof(VParam));
				if (!object->pParam)
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}
				memset(object->pParam, 0, sizeof(VParam));

				object->pParam->parameter = VCAL_PARAM_CHARSET;
				object->pParam->paramValue = 0x01 << VCAL_CHARSET_PARAM_UTF_8;
				object->pParam->pNext = NULL;

				object->pParam->pNext = (VParam*)malloc(sizeof(VParam));
				if (!object->pParam->pNext)
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}
				memset(object->pParam->pNext, 0, sizeof(VParam));

				object->pParam->pNext->parameter = VCAL_PARAM_ENCODING;
				object->pParam->pNext->paramValue = 0x01<<VCAL_ENC_PARAM_BASE64;


				object->pszValue[0] = (char*)malloc(str_len*2 + 1 );
				if ( !(object->pszValue[0]) )
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}

				if (NULL != sch->summary)
				{
					memset(object->pszValue[0], 0, str_len + 1);
					memcpy( object->pszValue[0], sch->summary, str_len);
				}
				CALS_DBG("sch->summary = %s,%s\n", sch->summary,object->pszValue[0]);

				object->valueCount = 1;
			}
			else
			{
				CAL_FREE(object);

				return true;
			}
		}
		break;

	case VCAL_TYPE_DESCRIPTION:
		{
			CALS_DBG("---__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_DESCRIPTION/ -------------\n");

			if (NULL != sch->description && strlen(sch->description) != 0)
			{
				object->property = VCAL_TYPE_DESCRIPTION;
				str_len = strlen(sch->description);
				//str_len = (str_len>500)?500:str_len;

				object->pParam = (VParam*)malloc(sizeof(VParam));
				if (!object->pParam)
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}
				memset(object->pParam, 0, sizeof(VParam));

				object->pParam->parameter = VCAL_PARAM_CHARSET;
				object->pParam->paramValue = 0x01 << VCAL_CHARSET_PARAM_UTF_8;
				object->pParam->pNext = NULL;

				object->pParam->pNext = (VParam*)malloc(sizeof(VParam));
				if (!object->pParam->pNext)
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}
				memset(object->pParam->pNext, 0, sizeof(VParam));

				object->pParam->pNext->parameter = VCAL_PARAM_ENCODING;
				object->pParam->pNext->paramValue = 0x01<<VCAL_ENC_PARAM_BASE64;

				object->pszValue[0] = (char*)malloc(str_len + 1 );
				if ( !(object->pszValue[0]) )
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}

				if (sch->description)
				{
					memset(object->pszValue[0], 0, str_len + 1);
					memcpy( object->pszValue[0], sch->description, str_len);
				}
				object->valueCount = 1;

				CALS_DBG("description = %s", object->pszValue[0]);

			}
			else
			{
				CAL_FREE(object);

				return true;
			}

		}
		break;
	case VCAL_TYPE_LOCATION:
		{
			CALS_DBG("-------------------__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_LOCATION -------------\n");

			if (NULL != sch->location && strlen(sch->location) != 0)
			{
				object->property = VCAL_TYPE_LOCATION;
				str_len = strlen(sch->location);
				object->pParam = (VParam*)malloc(sizeof(VParam));
				if (!object->pParam)
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}
				memset(object->pParam, 0, sizeof(VParam));

				object->pParam->parameter = VCAL_PARAM_CHARSET;
				object->pParam->paramValue = 0x01 << VCAL_CHARSET_PARAM_UTF_8;
				object->pParam->pNext = NULL;

				object->pParam->pNext = (VParam*)malloc(sizeof(VParam));
				if (!object->pParam->pNext)
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}
				memset(object->pParam->pNext, 0, sizeof(VParam));

				object->pParam->pNext->parameter = VCAL_PARAM_ENCODING;
				object->pParam->pNext->paramValue = 0x01;

				object->pszValue[0] = (char*)malloc(str_len + 1 );
				if ( !(object->pszValue[0]) )
				{
					vcal_free_vtree_memory( tree );
					CAL_FREE(object);

					return false;
				}

				if (sch->location)
				{
					memset(object->pszValue[0], 0, str_len + 1);
					memcpy( object->pszValue[0], sch->location, str_len);
				}
				object->valueCount = 1;
			}
			else
			{
				CAL_FREE(object);

				return true;
			}

		}
		break;
	case VCAL_TYPE_DTSTART:
		{
			CALS_DBG("-------------------__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_DTSTART -------------\n");

			object->property = VCAL_TYPE_DTSTART;

			object->pszValue[0] = (char*)malloc(VCALENDAR_TIME_STR_LEN + 1);
			if ( !(object->pszValue[0]) )
			{
				vcal_free_vtree_memory( tree );
				CAL_FREE(object);

				return false;
			}

			cal_vcalendar_convert_tm_to_vdata_str(&sch->start_date_time, object->pszValue[0]);

			object->valueCount = 1;
		}
		break;

	case VCAL_TYPE_DTEND:
		{
			CALS_DBG("-------------------__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_DTEND -------------\n");

			if(sch->cal_type == CAL_EVENT_TODO_TYPE)
			{
				break;
			}

			object->property = VCAL_TYPE_DTEND;

			object->pszValue[0] = (char*)malloc(VCALENDAR_TIME_STR_LEN + 1);
			if ( !(object->pszValue[0]) )
			{
				vcal_free_vtree_memory( tree );
				CAL_FREE(object);

				return false;
			}

			cal_vcalendar_convert_tm_to_vdata_str(&sch->end_date_time, object->pszValue[0]);

			object->valueCount = 1;
		}
		break;

		// in Calendar sch data structure, there is no due data field
		// end_date_time is assigned to due date
	case VCAL_TYPE_DUE:
		{
			CALS_DBG( "-------------------__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_DUE -------------\n");

			if(sch->cal_type != CAL_EVENT_TODO_TYPE)
			{
				break;
			}

			object->property = VCAL_TYPE_DUE;

			object->pszValue[0] = (char*)malloc(VCALENDAR_TIME_STR_LEN + 1);
			if ( !(object->pszValue[0]) )
			{
				vcal_free_vtree_memory( tree );
				CAL_FREE(object);

				return false;
			}
			cal_vcalendar_convert_tm_to_vdata_str(&sch->end_date_time, object->pszValue[0]);

			object->valueCount = 1;
		}
		break;

	case VCAL_TYPE_LAST_MODIFIED:
		{
			CALS_DBG( "-------------------__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_LAST_MODIFIED -------------\n");

			object->property = obj_type;
			object->pszValue[0] = (char*)malloc(VCALENDAR_TIME_STR_LEN + 1);
			if ( !(object->pszValue[0]) )
			{
				vcal_free_vtree_memory( tree );
				CAL_FREE(object);

				return false;
			}

			// get current time
			struct tm now;

			now.tm_year = 105;
			now.tm_mon = 2;
			now.tm_mday = 4;
			now.tm_hour = 10;
			now.tm_min = 12;
			now.tm_sec = 34;

			cal_vcalendar_convert_tm_to_vdata_str(&now, object->pszValue[0]);

			object->valueCount = 1;
		}
		break;

	case VCAL_TYPE_AALARM:
		{
			CALS_DBG( "\n-------------------__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_AALARM -------------\n");

			if (sch->alarm_list)
			{
				object->property = VCAL_TYPE_AALARM;
				cal_alarm_info_t *tmp_alarm = ((cal_value *)sch->alarm_list->data)->user_data;
				/*
				// set alarm type
				object->pParam = (VParam*)malloc(sizeof(VParam));
				if ( !object->pParam)
				{
					vcal_free_vtree_memory( tree );
					if (object != NULL)
					{
						CAL_FREE(object);
						object = NULL;
					}
					return false;
				}
				memset(object->pParam, 0, sizeof(VParam));

				object->pParam->parameter = VCAL_PARAM_TYPE;

				object->pParam->paramValue = 0x01 << VCAL_TYPE_PARAM_WAVE;

				// set alarm value
				object->pParam->pNext = (VParam*)malloc(sizeof(VParam));
				if ( !object->pParam->pNext)
				{
					vcal_free_vtree_memory( tree );
					if (object != NULL)
					{
						CAL_FREE(object);
						object = NULL;
					}
					return false;
				}

				memset(object->pParam->pNext, 0, sizeof(VParam));

				object->pParam->pNext->parameter = VCAL_PARAM_VALUE;
				object->pParam->pNext->paramValue = 0x01 << VCAL_VALUE_PARAM_URL;
				*/

				object->pszValue[0] = (char*)malloc(VCALENDAR_TIME_STR_LEN + 1);
				if ( !(object->pszValue[0]) )
				{
					vcal_free_vtree_memory( tree );
					if (object != NULL)
					{
						free(object);
						object = NULL;
					}
					return false;
				}
				struct tm ttm;

				// TODO: handle alarms(this is one alarm)
				struct tm *tm = __cal_vcalendar_compute_pre_time((struct tm*)&sch->start_date_time,
											&tmp_alarm->alarm_time, tmp_alarm->remind_tick, tmp_alarm->remind_tick_unit, &ttm);
				cal_vcalendar_convert_tm_to_vdata_str(tm, object->pszValue[0]);

				// set audio reminder file
				/*
				object->pszValue[1] = (char*)malloc(FM_FILENAME_LEN_MAX + 1 );
				if (!(object->pszValue[1]))
				{
					vcal_free_vtree_memory( tree );
					if (object != NULL)
					{
						CAL_FREE(object);
						object = NULL;
					}
					return false;
				}*/

				//memset(object->pszValue[1], 0,  FM_FILENAME_LEN_MAX + 1);
				//memcpy( object->pszValue[1], sch->alarm_tone, FM_FILENAME_LEN_MAX);

				object->valueCount = 1;
			}
			else
			{
				CAL_FREE(object);

				return true;
			}
		}
		break;

	case VCAL_TYPE_RRULE:
		{
			CALS_DBG( "\n-------------------__cal_vcalendar_sch_vtree_add_object /VCAL_TYPE_RRULE -------------\n");

			if (sch->repeat_term != CAL_REPEAT_NONE)
			{
				CALS_DBG(" ------------------------ begine to create RRULE-------------------------- ");

				object->property = VCAL_TYPE_RRULE;

				char 		repeat_end_date[VCALENDAR_TIME_STR_LEN + 1] = {0};
				char 		week_day_string[VCALENDAR_TIME_STR_LEN + 1] = {0};
				cal_vcalendar_convert_tm_to_vdata_str(&sch->repeat_end_date, repeat_end_date);

				CALS_DBG(" repeat_end_date = %s ", repeat_end_date);

				char str[100];
				memset(str, 0, 100);

				switch (sch->repeat_term)
				{
				case CAL_REPEAT_EVERY_DAY:
					sprintf(str, "D%d %s", sch->repeat_interval, repeat_end_date);
					break;

				case CAL_REPEAT_EVERY_WEEK:
				case CAL_REPEAT_EVERY_WEEKDAYS:
					__cal_vcalendar_get_week_day_string((struct tm*)&(sch->start_date_time), week_day_string);
					sprintf(str, "W%d %s %s", sch->repeat_interval, week_day_string, repeat_end_date);
					break;

				case CAL_REPEAT_EVERY_MONTH:
				case CAL_REPEAT_EVERY_MONTH_DAY:
					sprintf(str, "MD%d %s", sch->repeat_interval, repeat_end_date);
					break;

				case CAL_REPEAT_EVERY_YEAR:
				case CAL_REPEAT_EVERY_YEAR_DAY:
					sprintf(str, "YM%d %s", sch->repeat_interval, repeat_end_date);
					break;

				default:
					break;
				}
				object->valueCount = 1;

				object->pszValue[0] = (char*)malloc(strlen(str) + 1);
				if ( !(object->pszValue[0]) )
				{
					vcal_free_vtree_memory(tree);
					CAL_FREE(object);

					return false;
				}
				memset(object->pszValue[0], 0, (strlen(str) + 1));
				strcpy(object->pszValue[0], str);

				CALS_DBG("RRULE = %s", object->pszValue[0]);

			}
			else
			{
				CAL_FREE(object);

				return true;
			}
		}
		break;

	case VCAL_TYPE_PRIORITY:
		{
			CALS_DBG( "\n-------------------__cal_vcalendar_todo_vtree_add_object /VCAL_TYPE_PRIORITY -------------\n");

			object->property = VCAL_TYPE_PRIORITY;

			char str[3];
			memset(str, 0, 3);

			sprintf(str, "%d", sch->priority);

			object->pszValue[0] = (char*)malloc(3);
			if ( !(object->pszValue[0]) )
			{
				vcal_free_vtree_memory( tree );
				CAL_FREE(object);

				return false;
			}

			memset(object->pszValue[0], 0, 3);
			strcpy(object->pszValue[0], str);

			object->valueCount = 1;
		}
		break;

	case VCAL_TYPE_STATUS:
		{
			CALS_DBG( "\n-------------------__cal_vcalendar_todo_vtree_add_object /VCAL_TYPE_STATUS -------------\n");

			if(sch->cal_type != CAL_EVENT_TODO_TYPE)
			{
				break;
			}

			object->property = VCAL_TYPE_STATUS;

			object->pszValue[0] = (char*)malloc(20);
			if ( !(object->pszValue[0]) )
			{
				vcal_free_vtree_memory( tree );
				CAL_FREE(object);

				return false;
			}

			memset(object->pszValue[0], 0, 20);

			switch (sch->task_status)
			{
			case CALS_EVENT_STATUS_TENTATIVE:
				strcpy(object->pszValue[0], "TENTATIVE");
				break;
			case CALS_EVENT_STATUS_CONFIRMED:
				strcpy(object->pszValue[0], "CONFIRMED");
				break;
			case CALS_EVENT_STATUS_CANCELLED:
				strcpy(object->pszValue[0], "CANCELLED");
				break;
			case CALS_TODO_STATUS_NEEDS_ACTION:
				strcpy(object->pszValue[0], "NEEDS-ACTION");
				break;
			case CALS_TODO_STATUS_COMPLETED:
				strcpy(object->pszValue[0], "COMPLETED");
				break;
			case CALS_TODO_STATUS_IN_PROCESS:
				strcpy(object->pszValue[0], "IN-PROCESS");
				break;
			case CALS_TODO_STATUS_CANCELLED:
				strcpy(object->pszValue[0], "CANCELLED");
				break;
			default:
				strcpy(object->pszValue[0], "NEEDS-ACTION");
				break;
			}

			object->valueCount = 1;
			break;
		}

	default:
		break;

	}

	// if current is the header of the list
	if (tree->pTop == NULL)
	{
		CALS_DBG( "\n--------tree->pTop = object--------------\n");
		tree->pTop = object;
	}
	else
	{
		tree->pCur->pSibling = object;
	}

	// the object is the current object
	tree->pCur = object;

#endif
	return true;
}

#if 0
	bool
__cal_vnote_sch_vtree_add_object(VTree * tree, const cal_sch_full_t * sch, vCalType obj_type)
{
	VObject*		object;

	assert((tree != NULL) && (sch != NULL));

	object = (VObject*)malloc(sizeof(VObject));
	if ( !object )
	{
		VNoteFreeVTreeMemory( tree );
		return false;
	}

	memset( object, 0, sizeof(VObject));

	switch(obj_type)
	{
	case VNOTE_TYPE_BODY:
		{
			INFO("---__cal_vnote_sch_vtree_add_object /VNOTE_TYPE_BODY -------------\n");

#if 1
			if (strlen(sch->description) != 0)
			{
				object->property = VNOTE_TYPE_BODY;

				object->pParam = (VParam*)malloc(sizeof(VParam));
				if ( !object->pParam)
				{
					VNoteFreeVTreeMemory( tree );
					if (object != NULL)
					{
						CAL_FREE(object);
						object = NULL;
					}
					return false;
				}
				memset(object->pParam, 0, sizeof(VParam));
				memcpy( object->pszValue[0], sch->summary, CAL_SCH_SUMMARY_LEN_MAX);


				//				object->pParam->parameter = VNOTE_PARAM_ENCODING;
				//				object->pParam->paramValue = 0x01 << VNOTE_ENC_PARAM_QUOTED_PRINTABLE;

				//				object->pszValue[0] = (char*)malloc(CAL_SCH_DESCRIPTION_LEN_MAX + 1 );
				if ( !(object->pszValue[0]) )
				{
					VNoteFreeVTreeMemory( tree );
					CAL_FREE(object);

					return false;
				}

				//				memset(object->pszValue[0], 0, CAL_SCH_DESCRIPTION_LEN_MAX + 1);
				memcpy( object->pszValue[0], sch->description, CAL_SCH_DESCRIPTION_LEN_MAX);

				object->valueCount = 1;

			}
			else
			{
				CAL_FREE(object);

				return true;
			}

#else

			if (NULL != sch->description && strlen(sch->description) != 0)
			{
				object->property = VNOTE_TYPE_BODY;

				object->pszValue[0] = (char*)malloc(CAL_SCH_DESCRIPTION_LEN_MAX + 1 );
				if ( !(object->pszValue[0]) )
				{
					VNoteFreeVTreeMemory( tree );
					if (object != NULL)
					{
						CAL_FREE(object);
						object = NULL;
					}
					return false;
				}

				if (sch->description)
				{
					memset(object->pszValue[0], 0, CAL_SCH_DESCRIPTION_LEN_MAX + 1);
					memcpy( object->pszValue[0], sch->description, CAL_SCH_DESCRIPTION_LEN_MAX);
				}
				INFO("sch->description = %s\n", sch->description);

				object->valueCount = 1;
			}
			else
			{
				if (object != NULL)
				{
					CAL_FREE(object);
					object = NULL;
				}
				return true;
			}
#endif

		}
		break;

	case VNOTE_TYPE_LAST_MODIFIED:
		{
			INFO("---__cal_vnote_sch_vtree_add_object /VNOTE_TYPE_LAST_MODIFIED -------------\n");

			object->property = VNOTE_TYPE_LAST_MODIFIED;
			object->pszValue[0] = (char*)malloc(VCALENDAR_TIME_STR_LEN + 1);
			if ( !(object->pszValue[0]) )
			{
				VNoteFreeVTreeMemory( tree );
				CAL_FREE(object);

				return false;
			}

			// get current time
			struct tm now;

			now.tm_year = 105;
			now.tm_mon = 2;
			now.tm_mday = 4;
			now.tm_hour = 10;
			now.tm_min = 12;
			now.tm_sec = 34;

			cal_vcalendar_convert_tm_to_vdata_str(&now, object->pszValue[0]);

			object->valueCount = 1;
		}
		break;

	default:
		break;

	}

	// if current is the header of the list
	if (tree->pTop == NULL)
	{
		INFO( "\n--------tree->pTop = object--------------\n");
		tree->pTop = object;
	}
	else
	{
		tree->pCur->pSibling = object;
	}

	// the object is the current object
	tree->pCur = object;

	INFO( "\n-------------------exit __cal_vnote_sch_vtree_add_object --------------------\n");

	return true;
}

#endif

bool _cal_convert_sch_to_vcalendar(const cal_sch_full_t *sch_array,
	const int sch_count, char **vcal, cal_vCal_ver_t version)
{
#if 1
	int 	i;
	int 	j;

	VTree *cal_tree = NULL;
	VTree *tmp_tree = NULL;

	vCalType	obj_type[] =
	{
		VCAL_TYPE_SUMMARY,
		VCAL_TYPE_DESCRIPTION,
		VCAL_TYPE_LOCATION,
		VCAL_TYPE_DTSTART,
		VCAL_TYPE_DTEND,
		VCAL_TYPE_LAST_MODIFIED,
		VCAL_TYPE_ATTACH,
		VCAL_TYPE_CATEGORIES,
		VCAL_TYPE_PRIORITY,
		VCAL_TYPE_STATUS,
		VCAL_TYPE_AALARM,
		VCAL_TYPE_RRULE,
		0
	};


	assert((sch_array != NULL) && (vcal != NULL));


	cal_tree = (VTree*)malloc(sizeof(VTree));
	if ( !cal_tree )
		return false;

	memset( cal_tree, 0, sizeof(VTree));

	cal_tree->treeType = VCALENDAR;
	cal_tree->pTop = NULL;
	cal_tree->pCur = NULL;
	cal_tree->pNext = NULL;

	VTree *tree = cal_tree;

	for ( i = 0; i < sch_count; i++)
	{
		tmp_tree = (VTree*)malloc(sizeof(VTree));
		if ( !tmp_tree )
		{
			vcal_free_vtree_memory( cal_tree );
			return false;
		}

		memset( tmp_tree, 0, sizeof(VTree));

		if((sch_array + i)->cal_type == CAL_EVENT_TODO_TYPE){
			tmp_tree->treeType = VTODO;
			obj_type[4]=VCAL_TYPE_DUE;
		}
		else
		{
			tmp_tree->treeType = VEVENT;
		}

		tmp_tree->pTop = NULL;
		tmp_tree->pCur = NULL;
		tmp_tree->pNext = NULL;

		j=0;
		while (obj_type[j])
		{
			if ( !__cal_vcalendar_sch_vtree_add_object(tmp_tree, sch_array + i, obj_type[j]) )
			{
				vcal_free_vtree_memory(cal_tree);
				return false;
			}
			j++;
		}

		tree->pNext = tmp_tree;
		tree = tmp_tree;

	}

	tree->pNext = NULL;

	*vcal = vcal_encode( cal_tree );
	if ( *vcal == NULL )
	{
		ERR("vcal_encode Failed");
		vcal_free_vtree_memory( cal_tree );
		return false;
	}

	vcal_free_vtree_memory( cal_tree );
	CALS_DBG( "\n---------------------------exit _cal_convert_sch_to_vcalendar--------- -------------\n");
#endif
	return true;
}

	bool
_cal_convert_vcalendar_to_cal_data(const char *vcal, cal_sch_full_t **sch_array, int *sch_count)
{
	VTree*		vCal = NULL;
	VTree*		tmp_tree = NULL;
	VObject*		object = NULL;
	int 			error_code = 0;
	bool		start_date_exist = false;
	char*		pszAlarmValue = NULL;
	bool			is_ical = false;

	assert((vcal != NULL) && (sch_array != NULL));

	/* decode VCalendar */
	vCal = vcal_decode ( (char*)vcal );
	if ( vCal == NULL )
	{
		ERR( "vcal_decode() Failed");
		return false;
	}

	tmp_tree = vCal;
	int i = 0;

	*sch_array = malloc(sizeof(cal_sch_full_t));
	if(NULL == *sch_array)
	{
		ERR("malloc() Failed");
		return FALSE;
	}

	error_code = cals_init_full_record(*sch_array);

	while(tmp_tree != NULL )
	{

		char buff[MAX_BUFFER_SIZE];
		if (i > 0)
		{
			*sch_array = realloc(*sch_array, sizeof(cal_sch_full_t) * (i + 1));
			if(NULL == *sch_array)
			{
				ERR("realloc() Failed");
				return false;
			}
			error_code = cals_init_full_record(*sch_array + i);
		}

		memset(buff, 0, MAX_BUFFER_SIZE);

		if (tmp_tree->treeType == VTODO)
		{
			((*sch_array) + i)->cal_type = CAL_EVENT_TODO_TYPE;
		}
		else if (tmp_tree->treeType == VEVENT)
		{
			((*sch_array) + i)->cal_type = CAL_EVENT_SCHEDULE_TYPE;
		}
		/*
			else if ((tmp_tree->treeType == STANDARD) || (tmp_tree->treeType == DAYLIGHT))
			{
			object = tmp_tree->pTop;
			while (true)
			{
			if (object->property == VCAL_TYPE_TZOFFSETFROM)
			{
			((*sch_array) + i)->timezone = __cal_vcalendar_get_timezone(object->pszValue[0], (char**)timezoneoffset, 33);
			}

			if ( object->pSibling != NULL )
			{
			object = object->pSibling;
			}
			else
			{
			break;
			}
			}
			tmp_tree = tmp_tree->pNext;

			continue;
			}*/
			else if (tmp_tree->treeType == VCALENDAR)
			{
				object = tmp_tree->pTop;
				while (true)
				{
					if (object->property == VCAL_TYPE_VERSION)
					{
						if (strcmp(object->pszValue[0], "2.0") == 0)
						{
							is_ical = true;
						}
					}

					if ( object->pSibling != NULL )
					{
						object = object->pSibling;
					}
					else
					{
						break;
					}
				}
				tmp_tree = tmp_tree->pNext;

				continue;
			}
			else
			{
				tmp_tree = tmp_tree->pNext;
				continue;
			}

			// initialize a sch
			((*sch_array) + i)->start_date_time.tm_year = 71;
			((*sch_array) + i)->start_date_time.tm_mon = 1;
			((*sch_array) + i)->start_date_time.tm_mday = 1;
			((*sch_array) + i)->start_date_time.tm_hour = 0;
			((*sch_array) + i)->start_date_time.tm_min = 0;
			((*sch_array) + i)->start_date_time.tm_sec = 0;

			((*sch_array) + i)->end_date_time.tm_year = 71;
			((*sch_array) + i)->end_date_time.tm_mon = 1;
			((*sch_array) + i)->end_date_time.tm_mday = 1;
			((*sch_array) + i)->end_date_time.tm_hour = 0;
			((*sch_array) + i)->end_date_time.tm_min = 0;
			((*sch_array) + i)->end_date_time.tm_sec = 0;

			((*sch_array) + i)->repeat_end_date.tm_year = 137;
			((*sch_array) + i)->repeat_end_date.tm_mon = 11;
			((*sch_array) + i)->repeat_end_date.tm_mday = 31;
			((*sch_array) + i)->repeat_end_date.tm_hour = 23;
			((*sch_array) + i)->repeat_end_date.tm_min = 59;
			((*sch_array) + i)->repeat_end_date.tm_sec = 59;

			(*sch_array + i)->sch_category = CAL_SCH_APPOINTMENT;

			object = tmp_tree->pTop;

			// read vcalendar and fill sch structure
			while (true)
			{
				CALS_DBG(" \nproperty=%d \n",object->property);

				switch (object->property)
				{
				case VCAL_TYPE_CATEGORIES:

					//(*sch_array + i)->cal_type = CAL_EVENT_SCHEDULE_TYPE;

					CALS_DBG("sch_category = %s", object->pszValue[0]);

					if (strcmp(object->pszValue[0], "APPOINTMENT") == 0)
					{
						(*sch_array + i)->sch_category = CAL_SCH_APPOINTMENT;

					}
					else if (strcmp(object->pszValue[0], "HOLIDAY") == 0)
					{
						(*sch_array + i)->sch_category = CAL_SCH_HOLIDAY;

					}
					else if (strcmp(object->pszValue[0], "BUSINESS") == 0)
					{
						(*sch_array + i)->sch_category = CAL_SCH_BUSSINESS;

					}
					else if (strcmp(object->pszValue[0], "SPECIAL OCCASION") == 0)
					{
						(*sch_array + i)->sch_category = CAL_SCH_SPECIAL_OCCASION;

					}
					else if (strcmp(object->pszValue[0], "IMPORTANT") == 0)
					{
						(*sch_array + i)->sch_category = CAL_SCH_IMPORTANT;

					}
					else if (strcmp(object->pszValue[0], "BIRTHDAY") == 0)
					{
						(*sch_array + i)->sch_category = CAL_SCH_BIRTHDAY;

					}

					else if (strcmp(object->pszValue[0], "MISCELLANEOUS") == 0)
					{
						(*sch_array + i)->sch_category = CAL_SCH_APPOINTMENT;

					}
					else
					{
						(*sch_array + i)->sch_category = CAL_SCH_APPOINTMENT;

					}

					break;

				case VCAL_TYPE_SUMMARY:
					(*sch_array + i)->summary = strdup(object->pszValue[0]);
					//				memcpy((*sch_array + i)->summary, object->pszValue[0], CAL_SCH_SUMMARY_LEN_MAX);
					break;
				case VCAL_TYPE_DESCRIPTION:
					(*sch_array + i)->description = strdup(object->pszValue[0]);
					//memcpy( (*sch_array + i)->description, object->pszValue[0], CAL_SCH_DESCRIPTION_LEN_MAX);
					break;
				case VCAL_TYPE_LOCATION:
					(*sch_array + i)->location = strdup(object->pszValue[0]);
					//				memcpy( (*sch_array + i)->location, object->pszValue[0], CAL_SCH_LOCATION_LEN_MAX);
					break;

					/*			case VCAL_TYPE_ATTACH:
								memcpy( (*sch_array + i)->attach_path, object->pszValue[0], FM_FILENAME_LEN_MAX);
								break;*/

				case VCAL_TYPE_DTSTART:
					cal_vcalendar_convert_utc_str_to_tm(object->pszValue[0], &(*sch_array + i)->start_date_time);
					start_date_exist = true;
					break;
				case VCAL_TYPE_DTEND:
					cal_vcalendar_convert_utc_str_to_tm(object->pszValue[0], &(*sch_array + i)->end_date_time);
					break;
				case VCAL_TYPE_CREATED:
					cal_vcalendar_convert_utc_str_to_tm(object->pszValue[0], &(*sch_array + i)->created_date_time);
					break;
				case VCAL_TYPE_COMPLETED:
					cal_vcalendar_convert_utc_str_to_tm(object->pszValue[0], &(*sch_array + i)->completed_date_time);
					break;
				case VCAL_TYPE_DUE:
					cal_vcalendar_convert_utc_str_to_tm(object->pszValue[0], &(*sch_array + i)->end_date_time);
					break;
				case VCAL_TYPE_LAST_MODIFIED:
					cal_vcalendar_convert_utc_str_to_tm(object->pszValue[0], &(*sch_array + i)->last_modified_time);

					break;
				case VCAL_TYPE_RRULE:
					CALS_DBG( " \n---------------------VCAL_TYPE_RRULE is_ical is %d-------------------- \n",is_ical);

					// Calendar sch structure has no repeat interval field, discard it
					struct tm tm;
					tm.tm_year = 137;
					tm.tm_mon = 11;
					tm.tm_mday = 31;
					tm.tm_hour = 23;
					tm.tm_min = 59;
					tm.tm_sec = 59;
					int count = -1;

					if(is_ical)
					{
						__cal_vcalendar_parse_sch_repeat_rule_for_ical(object,&(*sch_array + i)->repeat_term, &(*sch_array + i)->repeat_interval, &tm);
					}
					else
					{
						__cal_vcalendar_parse_sch_repeat_rule( object->pszValue[0], &(*sch_array + i)->repeat_term, &(*sch_array + i)->repeat_interval, &tm,&count);
					}

					if(count != -1)
					{
						(*sch_array + i)->repeat_occurrences = count;
					}

					((*sch_array + i)->repeat_end_date).tm_year = tm.tm_year;
					((*sch_array + i)->repeat_end_date).tm_mon = tm.tm_mon;
					((*sch_array + i)->repeat_end_date).tm_mday = tm.tm_mday;
					((*sch_array + i)->repeat_end_date).tm_hour = ((*sch_array + i)->end_date_time).tm_hour;
					((*sch_array + i)->repeat_end_date).tm_min = ((*sch_array + i)->end_date_time).tm_min;
					((*sch_array + i)->repeat_end_date).tm_sec = ((*sch_array + i)->end_date_time).tm_sec;

					if (((*sch_array + i)->repeat_term == CAL_REPEAT_EVERY_WEEK) || ((*sch_array + i)->repeat_term == CAL_REPEAT_EVERY_WEEKDAYS))
					{
						(*sch_array + i)->week_flag = malloc(sizeof(char) * (DAY_OF_A_WEEK+1));
						memset((*sch_array + i)->week_flag,0x00,sizeof(char) * (DAY_OF_A_WEEK+1));
						cal_db_service_set_sch_weekflag(&((*sch_array + i)->start_date_time), (*sch_array + i)->week_flag);
					}
					CALS_DBG( " \n repeat_end_date->year = %d\n", ((*sch_array + i)->repeat_end_date).tm_year);
					CALS_DBG( " \n repeat_end_date->mon = %d\n", ((*sch_array + i)->repeat_end_date).tm_mon);
					CALS_DBG( " \n repeat_end_date->mday = %d\n", ((*sch_array + i)->repeat_end_date).tm_mday);

					break;

					// Note: Before decode AALARM, decode START_DATE_TIME first!
				case VCAL_TYPE_AALARM:
					CALS_DBG(  " \n---------------------VCAL_TYPE_AALARM-------------------- \n");
					pszAlarmValue = object->pszValue[0];
					//__cal_vcalendar_parse_reminder_time(object->pszValue[0],  &(*sch_array + i)->start_date_time, &(*sch_array + i)->remind_tick, &(*sch_array + i)->remind_tick_unit);

					/*if (object->pszValue[1] != NULL)
					  {
					  strncpy((*sch_array + i)->alarm_tone, object->pszValue[1], FM_FILENAME_LEN_MAX);
					  }*/
					break;

					// sch data structure has no priority and status fields
				case VCAL_TYPE_PRIORITY:
					CALS_DBG(  " \n---------------------VCAL_TYPE_PRIORITY-------------------- \n");
					if (object->pszValue[0] != NULL)
					{

						int prio = atoi(object->pszValue[0]);
						if ((prio < 3) && (prio >= 0))
						{
							(*sch_array + i)->priority = prio;
						}
						else
						{
							//						INFO(   " \n---------------------VCAL_TYPE_PRIORITY-HHHHHHHHH------------------- \n");

							(*sch_array + i)->priority = CAL_PRIORITY_HIGH;
						}

					}

					break;

				case VCAL_TYPE_STATUS:
					CALS_DBG(  " \n---------------------VCAL_TYPE_STATUS-------------------- \n");

					if ( object->pszValue[0] != NULL)
					{
						if (CAL_EVENT_TODO_TYPE == (*sch_array + i)->cal_type) {
							if (strcmp(object->pszValue[0], "NEEDS-ACTION") == 0)
								(*sch_array + i)->task_status = CALS_TODO_STATUS_NEEDS_ACTION;
							else if (strcmp(object->pszValue[0], "COMPLETED") == 0)
								(*sch_array + i)->task_status = CALS_TODO_STATUS_COMPLETED;
							else if (strcmp(object->pszValue[0], "IN-PROCESS") == 0)
								(*sch_array + i)->task_status = CALS_TODO_STATUS_IN_PROCESS;
							else if (strcmp(object->pszValue[0], "CANCELLED") == 0)
								(*sch_array + i)->task_status = CALS_TODO_STATUS_CANCELLED;
							else
								(*sch_array + i)->task_status = CALS_STATUS_NONE;
						} else if (CAL_EVENT_SCHEDULE_TYPE == (*sch_array + i)->cal_type) {
							if (strcmp(object->pszValue[0], "TENTATIVE") == 0)
								(*sch_array + i)->task_status = CALS_EVENT_STATUS_TENTATIVE;
							else if (strcmp(object->pszValue[0], "CONFIRMED") == 0)
								(*sch_array + i)->task_status = CALS_EVENT_STATUS_CONFIRMED;
							else if (strcmp(object->pszValue[0], "CANCELLED") == 0)
								(*sch_array + i)->task_status = CALS_EVENT_STATUS_CANCELLED;
							else
								(*sch_array + i)->task_status = CALS_STATUS_NONE;
						}
					}
					break;

				default:
					break;

				}

				if ( object->pSibling != NULL )
				{
					object = object->pSibling;
				}
				else
				{
					break;
				}
			} /* while */

			if(false == start_date_exist)
			{
				memcpy(&(*sch_array + i)->start_date_time,&(*sch_array + i)->start_date_time,sizeof(struct tm));
			}


			if(((*sch_array + i)->start_date_time.tm_hour==0) && ((*sch_array + i)->start_date_time.tm_min==0) &&
					((*sch_array + i)->start_date_time.tm_sec==0) && ((*sch_array + i)->end_date_time.tm_hour==0) &&
					((*sch_array + i)->end_date_time.tm_min==0) && ((*sch_array + i)->end_date_time.tm_sec==0))
			{
				(*sch_array + i)->all_day_event = true;
			}

			if(pszAlarmValue && pszAlarmValue[0] != '\0')
			{
				cal_alarm_info_t *alarm;
				cal_value *val;

				val = calendar_svc_value_new(CAL_VALUE_LST_ALARM); // Alarm value create

				alarm = (cal_alarm_info_t *)val->user_data;
				__cal_vcalendar_parse_reminder_time(pszAlarmValue, (*sch_array + i)->sch_category,
						&(*sch_array + i)->start_date_time, &alarm->remind_tick, &alarm->remind_tick_unit);
				cal_vcalendar_convert_utc_str_to_tm(pszAlarmValue, &alarm->alarm_time);

				if (((*sch_array + i)->sch_category == CAL_SCH_SPECIAL_OCCASION) ||
						((*sch_array + i)->sch_category == CAL_SCH_BIRTHDAY)) {
					alarm->alarm_time.tm_year = (*sch_array + i)->start_date_time.tm_year;
					alarm->alarm_time.tm_mon = (*sch_array + i)->start_date_time.tm_mon;
					alarm->alarm_time.tm_mday = (*sch_array + i)->start_date_time.tm_mday;
				} else {
					cal_db_service_copy_struct_tm(&((*sch_array + i)->start_date_time), &alarm->alarm_time);
				}

				(*sch_array + i)->alarm_list = g_list_append((*sch_array + i)->alarm_list, val);

				CALS_DBG( "remind_tick = %d", alarm->remind_tick);
				CALS_DBG( "remind_tick_unit = %d", alarm->remind_tick_unit);
				pszAlarmValue = NULL;
			}

			/*		int len = strlen((*sch_array + i)->description);

					strncat((*sch_array + i)->description, buff, CAL_SCH_DESCRIPTION_LEN_MAX - len -10);*/

			tmp_tree = tmp_tree->pNext;

			i++;
	}

	*sch_count = i;

	/* Free VCalendar Tree */
	vcal_free_vtree_memory( vCal );
	return true;
}
