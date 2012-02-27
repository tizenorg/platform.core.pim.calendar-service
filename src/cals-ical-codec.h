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
#ifndef __CALENDAR_SVC_ICAL_CODEC_H__
#define __CALENDAR_SVC_ICAL_CODEC_H__

#define CR				0x0d
#define LF				0x0a
#define TAB				0x09
#define WSP				0x20
#define UNKNOWN_NAME 	0x80000000

#define VCALENDAR		0x02
#define VNOTE 			0x0b

#define VEVENT			0x03
#define VTODO			0x04
#define VJOURNAL		0x05
#define VFREEBUSY		0x06
#define VTIMEZONE		0x07
#define VALARM			0x08
#define VMESSAGE		0x0c
#define VBODY			0X0d
#define STANDARD		0x09
#define DAYLIGHT		0x0a

#define VTYPE_TOKEN_SEMICOLON		';'
#define VTYPE_TOKEN_COLON			':'
#define VTYPE_TOKEN_EQUAL			'='
#define VTYPE_TOKEN_COMMA			','
#define VTYPE_TOKEN_DOT				'.'
#define VTYPE_TOKEN_QUOTE			'\''
#define VTYPE_TOKEN_DBLQUOTE		'"'

#define VDATA_VALUE_COUNT_MAX	2000

/****************************************************************************************************/
/*											ENUMERATION DECLARATION									*/
/****************************************************************************************************/
/* Property */
typedef enum
{
	VCAL_TYPE_AALARM,
	VCAL_TYPE_ACTION,
	VCAL_TYPE_ATTACH,
	VCAL_TYPE_ATTENDEE,
	VCAL_TYPE_BEGIN,
	VCAL_TYPE_CALSCALE,
	VCAL_TYPE_CATEGORIES,
	VCAL_TYPE_CLASS,
	VCAL_TYPE_COMMENT,
	VCAL_TYPE_COMPLETED,
	VCAL_TYPE_CONTACT,
	VCAL_TYPE_CREATED,
	VCAL_TYPE_DALARM,
	VCAL_TYPE_DAYLIGHT,
	VCAL_TYPE_DCREATED,
	VCAL_TYPE_DESCRIPTION,
	VCAL_TYPE_DTEND,
	VCAL_TYPE_DTSTAMP,
	VCAL_TYPE_DTSTART,
	VCAL_TYPE_DUE,
	VCAL_TYPE_DURATION,
	VCAL_TYPE_END,
	VCAL_TYPE_EXDATE,
	VCAL_TYPE_EXRULE,
	VCAL_TYPE_FREEBUSY,
	VCAL_TYPE_GEO,
	VCAL_TYPE_LAST_MODIFIED,
	VCAL_TYPE_LOCATION,
	VCAL_TYPE_MALARM,
	VCAL_TYPE_METHOD,
	VCAL_TYPE_ORGANIZER,
	VCAL_TYPE_PALARM,
	VCAL_TYPE_PERCENT_COMPLETE,
	VCAL_TYPE_PRIORITY,
	VCAL_TYPE_PRODID,
	VCAL_TYPE_RDATE,
	VCAL_TYPE_RECURRENCE_ID,
	VCAL_TYPE_RELATED_TO,
	VCAL_TYPE_REPEAT,
	VCAL_TYPE_REQUEST_STATUS,
	VCAL_TYPE_RESOURCES,
	VCAL_TYPE_RNUM,
	VCAL_TYPE_RRULE,
	VCAL_TYPE_SEQUENCE,
	VCAL_TYPE_STANDARD,
	VCAL_TYPE_STATUS,
	VCAL_TYPE_SUMMARY,
	VCAL_TYPE_TRANSP,
	VCAL_TYPE_TRIGGER,
	VCAL_TYPE_TZ,
	VCAL_TYPE_TZID,
	VCAL_TYPE_TZNAME,
	VCAL_TYPE_TZOFFSETFROM,
	VCAL_TYPE_TZOFFSETTO,
	VCAL_TYPE_TZURL,
	VCAL_TYPE_URL,
	VCAL_TYPE_UID,
	VCAL_TYPE_VALARM,
	VCAL_TYPE_VCALENDAR,
	VCAL_TYPE_VERSION,
	VCAL_TYPE_VEVENT,
	VCAL_TYPE_VFREEBUSY,
	VCAL_TYPE_VJOURNAL,
	VCAL_TYPE_VTIMEZONE,
	VCAL_TYPE_VTODO,
	VCAL_TYPE_ALLDAY
}vCalType;

#define VCAL_TYPE_NUM	66

/* Parameter */
typedef enum
{
	VCAL_PARAM_ALTREP,
	VCAL_PARAM_CHARSET,
	VCAL_PARAM_CN,
	VCAL_PARAM_CONTEXT,
	VCAL_PARAM_CUTYPE,
	VCAL_PARAM_DELEGATED_FROM,
	VCAL_PARAM_DELEGATED_TO,
	VCAL_PARAM_DIR,
	VCAL_PARAM_ENCODING,
	VCAL_PARAM_EXPECT,
	VCAL_PARAM_FBTYPE,
	VCAL_PARAM_FMTYPE,
	VCAL_PARAM_LANGUAGE,
	VCAL_PARAM_MEMBER,
	VCAL_PARAM_PARTSTAT,
	VCAL_PARAM_RANGE,
	VCAL_PARAM_RELATED,
	VCAL_PARAM_RELTYPE,
	VCAL_PARAM_ROLE,
	VCAL_PARAM_RSVP,
	VCAL_PARAM_SENT_BY,
	VCAL_PARAM_STATUS,
	VCAL_PARAM_TYPE,
	VCAL_PARAM_TZID,
	VCAL_PARAM_VALUE
}vCalendarParamName;

#define VCAL_PARAM_NUM		25

/* Cu type value */
typedef enum
{
	VCAL_CUTYPE_PARAM_GROUP,
	VCAL_CUTYPE_PARAM_INDIVIDUAL,
	VCAL_CUTYPE_PARAM_RESOURCE,
	VCAL_CUTYPE_PARAM_ROOM,
	VCAL_CUTYPE_PARAM_UNKNOWN
} vCalCutypeVal;

#define VCAL_CUTYPE_PARAM_NUM	5

/* Parameter encoding value */
typedef enum
{
	VCAL_ENC_PARAM_B,
	VCAL_ENC_PARAM_BASE64,
	VCAL_ENC_PARAM_QUOTED_PRINTABLE,
	VCAL_ENC_PARAM_7BIT,
	VCAL_ENC_PARAM_8BIT
}vCalEncVal;

#define VCAL_ENCODE_PARAM_NUM	5

/* Fb Type value */
typedef enum
{
	VCAL_FBTYPE_PARAM_BUSY,
	VCAL_FBTYPE_PARAM_BUSY_TENTATIVE,
	VCAL_FBTYPE_PARAM_BUSY_UNAVAILABLE,
	VCAL_FBTYPE_PARAM_FREE
} vCalFbtypeVal;

#define VCAL_FBTYPE_PARAM_NUM	4

/* Partstat value */
typedef enum
{
	VCAL_PARTSTAT_PARAM_ACCEPTED,
	VCAL_PARTSTAT_PARAM_COMPLETED,
	VCAL_PARTSTAT_PARAM_DELEGATED,
	VCAL_PARTSTAT_PARAM_DECLINED,
	VCAL_PARTSTAT_PARAM_IN_PROCESS,
	VCAL_PARTSTAT_PARAM_NEED_ACTION,
	VCAL_PARTSTAT_PARAM_TENTATIVE
} vCalPartstatVal;

#define VCAL_PARTSTAT_PARAM_NUM	7

/* Range value */
typedef enum
{
	VCAL_RANGE_PARAM_THISANDFUTURE,
	VCAL_RANGE_PARAM_THISANDPRIOR
} vCalRangeVal;

#define VCAL_RANGE_PARAM_NUM	2

/* Related value */
typedef enum
{
	VCAL_RELATED_PARAM_END,
	VCAL_RELATED_PARAM_START
} vCalRelatedVal;

#define VCAL_RELATED_PARAM_NUM	2

/* Rel type value */
typedef enum
{
	VCAL_RELTYPE_PARAM_CHILD,
	VCAL_RELTYPE_PARAM_PARENT,
	VCAL_RELTYPE_PARAM_SIBLING
} vCalReltypeVal;

#define VCAL_RELTYPE_PARAM_NUM	3

/* Value value */
typedef enum
{
	VCAL_VALUE_PARAM_BINARY,
	VCAL_VALUE_PARAM_BOOLEAN,
	VCAL_VALUE_PARAM_CAL_ADDRESS,
	VCAL_VALUE_PARAM_CID,
	VCAL_VALUE_PARAM_CONTENT_ID,
	VCAL_VALUE_PARAM_DATE,
	VCAL_VALUE_PARAM_DATE_TIME,
	VCAL_VALUE_PARAM_DURATION,
	VCAL_VALUE_PARAM_FLOAT,
	VCAL_VALUE_PARAM_INTEGER,
	VCAL_VALUE_PARAM_PERIOD,
	VCAL_VALUE_PARAM_PHONE_NUMBER,
	VCAL_VALUE_PARAM_RECUR,
	VCAL_VALUE_PARAM_TEXT,
	VCAL_VALUE_PARAM_TIME,
	VCAL_VALUE_PARAM_URI,
	VCAL_VALUE_PARAM_URL,
	VCAL_VALUE_PARAM_UTC_OFFSET,
	VCAL_VALUE_PARAM_VCALENDAR,
	VCAL_VALUE_PARAM_VEVENT,
	VCAL_VALUE_PARAM_VTODO
}vCalValVal;

#define VCAL_VALUE_PARAM_NUM		21

/* Parameter type value */
typedef enum
{
	VCAL_TYPE_PARAM_AIFF,
	VCAL_TYPE_PARAM_BBS,
	VCAL_TYPE_PARAM_CAR,
	VCAL_TYPE_PARAM_CELL,
	VCAL_TYPE_PARAM_DOM,
	VCAL_TYPE_PARAM_FAX,
	VCAL_TYPE_PARAM_GIF,
	VCAL_TYPE_PARAM_HOME,
	VCAL_TYPE_PARAM_INTL,
	VCAL_TYPE_PARAM_INTERNET,
	VCAL_TYPE_PARAM_ISDN,
	VCAL_TYPE_PARAM_JPEG,
	VCAL_TYPE_PARAM_MODEM,
	VCAL_TYPE_PARAM_MSG,
	VCAL_TYPE_PARAM_PAGER,
	VCAL_TYPE_PARAM_PARCEL,
	VCAL_TYPE_PARAM_PCM,
	VCAL_TYPE_PARAM_PCS,
	VCAL_TYPE_PARAM_PNG,
	VCAL_TYPE_PARAM_POSTAL,
	VCAL_TYPE_PARAM_PREF,
	VCAL_TYPE_PARAM_VCARD,
	VCAL_TYPE_PARAM_VIDEO,
	VCAL_TYPE_PARAM_VOICE,
	VCAL_TYPE_PARAM_WAVE,
	VCAL_TYPE_PARAM_WBMP,
	VCAL_TYPE_PARAM_WORK,
	VCAL_TYPE_PARAM_X400
}vCalTypeVal;

#define VCAL_TYPE_PARAM_NUM		28

/* Parameter expect value */
typedef enum
{
	VCAL_EXPECT_PARAM_FYI,
	VCAL_EXPECT_PARAM_IMMEDIATE,
	VCAL_EXPECT_PARAM_REQUEST,
	VCAL_EXPECT_PARAM_REQUIRE
}vCalExpectVal;

#define VCAL_EXPECT_PARAM_NUM		4

/* Parameter role value */
typedef enum
{
	VCAL_ROLE_PARAM_ATTENDEE,
	VCAL_ROLE_PARAM_CHAIR,
	VCAL_ROLE_PARAM_DELEGATE,
	VCAL_ROLE_PARAM_NON_PARTICIPANT,
	VCAL_ROLE_PARAM_OPT_PARTICIPANT,
	VCAL_ROLE_PARAM_ORGANIZER,
	VCAL_ROLE_PARAM_OWNER,
	VCAL_ROLE_PARAM_REQ_PARTICIPANT
}vCalRoleVal;

#define VCAL_ROLE_PARAM_NUM		8

/* Parameter RSVP value */
typedef enum
{
	VCAL_RSVP_PARAM_FALSE,
	VCAL_RSVP_PARAM_NO,
	VCAL_RSVP_PARAM_TRUE,
	VCAL_RSVP_PARAM_YES
}vCalRsvpVal;

#define VCAL_RSVP_PARAM_NUM		4

/* Parameter Charset value */
typedef enum
{
	VCAL_CHARSET_PARAM_UTF_8,
	VCAL_CHARSET_PARAM_UTF_16,
	VCAL_CHARSET_PARAM_SHIFT_JIS
}vCalCharsetVal;

#define VCAL_CHARSET_PARAM_NUM 2

/* Parameter STATUS value */
typedef enum
{
	VCAL_STATUS_PARAM_ACCEPTED,
	VCAL_STATUS_PARAM_COMPLETED,
	VCAL_STATUS_PARAM_CONFIRMED,
	VCAL_STATUS_PARAM_DECLINED,
	VCAL_STATUS_PARAM_DELEGATED,
	VCAL_STATUS_PARAM_NEEDS_ACTION,
	VCAL_STATUS_PARAM_SENT,
	VCAL_STATUS_PARAM_TENTATIVE
}vCalStatusVal;

#define VCAL_STATUS_PARAM_NUM		8


/* VCalendar encoder/decoder status */
#define VCAL_TYPE_NAME_STATUS	1
#define VCAL_PARAM_NAME_STATUS	2
#define VCAL_TYPE_VALUE_STATUS	3
#define VCAL_PARAM_VALUE_STATUS 4


/****************************************************************************************************/
/*									 GLOBAL STRUCTURE DECLARATION									*/
/****************************************************************************************************/

typedef struct _VParam VParam;
typedef struct _VObject VObject;
typedef struct _ValueObj ValueObj;
typedef struct _VTree VTree;

struct _VTree
{
	int			treeType;
	VObject*	pTop;
	VObject*	pCur;
	VTree*		pNext;
};

struct _VParam
{
	int			parameter;
	int			paramValue;
	VParam*		pNext;
};

struct _VObject
{
	int			property;
	VParam*		pParam;
	int			valueCount;
	int			numOfBiData;
	char*		pszValue[VDATA_VALUE_COUNT_MAX];
	VObject*	pSibling;
	VObject*	pParent;
	VObject*	pChild;

	char*		pszGroupName; //VDATA_GROUPNAME_SUPPORTED
};

struct _ValueObj
{
	char*		szName;
	int			flag;
};


#define FEATURE_SHIFT_JIS

/****************************************************************************************************/
/*										 FUNCTION DECLARATION										*/
/****************************************************************************************************/
int _VIsSpace( char );
int _VRLSpace( char * );
int _VRTSpace( char * );
int _VUnescape( char *);
int _VEscape(char*);
int _VManySpace2Space( char * );
int _VB64Decode( char *, char * );
int _VB64Encode( char *, char *, int );
int _VUnfolding( char * );
void _VFolding( char *, char * );
int _VQPDecode( char * );
int _VQPEncode( char *, char * );

void _VFoldingQP( char *result, char *contentline );
void _VFoldingNoSpace( char *result, char *contentline );
int _VManyCRLF2CRLF(char *pIn);
int _VUnfoldingNoSpec( char *string, int vType );


/**
* @fn VTree* vcal_decode(char* pVCalRaw);
* This function decodes a vCalendar raw buffer
* @return This function returns a pointer to VTree.
* @param[in] pVCalRaw  Points to the vCalendar raw buffers.
* @see vcal_encode
*/

VTree* vcal_decode(char *pVCalRaw);

/**
* @fn char* vcal_encode(VTree* pVTree);
* This function encodes a vTree data to vCalendar buffer
* @return This function returns a pointer to vCalendar buffer.
* @param[in] pVTree Points to vTree data.
* @see vcal_decode
*/

char *vcal_encode(VTree* pVTree);

/**
* @fn bool	vcal_free_vtree_memory( VTree* pTree );
* This function free vTree memory
* @return This function returns true if success and false if failuer.
* @param[in] pTree Points to  a vTree data.
* @see vcal_decode
*/

bool vcal_free_vtree_memory( VTree* pTree );

bool _cal_convert_sch_to_vcalendar(const cal_sch_full_t *sch_array, const int sch_count, char** vcal, cal_vCal_ver_t version );

bool _cal_convert_vcalendar_to_cal_data(const char *vcal, cal_sch_full_t **sch_array, int *sch_count);


#endif	/* __CALENDAR_SVC_ICAL_CODEC_H__ */
