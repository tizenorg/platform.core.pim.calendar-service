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
#include <stdlib.h>
#include <string.h>

#include "cals-typedef.h"
#include "cals-ical-codec.h"

#define VCARD_TYPE_NUM 34
#define VCAL_TYPE_NUM	66
#define VMSG_TYPE_NUM 12

/* BASE64 TABLE */
static const char Base64Table[65] = {							'A',
	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',
	'L',	'M',	'N',	'O',	'P',	'Q',	'R',	'S',	'T',	'U',
	'V',	'W',	'X',	'Y',	'Z',	'a',	'b',	'c',	'd',	'e',
	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',
	'z',	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',
	'9',	'+',	'/',	'='
};

/* Function Declaration */
static int __VFindBase64( char );
static int __VBase64Check( char * );
static char __VHexaDecoder( char * );
static void __VHexaEncoder( char * );
static int __VIsPrintable( char );


/**
 * vCardIsSpace() returns one if char is either a space, tab, or newline.
 *
 * @param      s1                [in] pointer to first string.
 * @param      s2                [in] pointer to second string.
 * @return     1                 'in' is a space character.
 * @return     0                 'in' is not a space.
 */
	int
_VIsSpace( char in )
{
	//DLOG(  "_VIsSpace() enter..\n");

	if ( ( in == TAB ) || ( in == WSP ) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


/*
 * vRemLeadSpace() removes leading space in string 'in'.
 *
 * @param      in                 [inout] pointer to string.
 * @return     0                  if success.
 */
	int
_VRLSpace( char *in )
{
	int			i, j;
	short int	done;

	//DLOG(  "_VRLSpace() enter..\n");

	i = 0;
	done = 0;

	while ( !done && in[i] )
	{
		if ( _VIsSpace( in[i] ) )
		{
			i++;
		}
		else
		{
			done = 1;
		}
	}

	j = 0;
	while ( in[i] )
	{
		in[j++] = in[i++];
	}

	in[j] = '\0';

	return 0;
}


/*
 * vRemTermSpace() removes terminating space.
 *
 * @param      in                 [inout] pointer to string.
 * @return     0                  if success.
 */
	int
_VRTSpace( char *in )
{
	int			i;
	short int	done;

	//DLOG(  "_VRTSpace() enter..\n");

	i = strlen(in) - 1;
	done = 0;

	while ( !done && !( i < 0 ) )
	{
		if ( _VIsSpace( in[i] ) )
		{
			in[i--] = '\0';
		}
		else
		{
			done = 1;
		}
	}

	return(0);
}


/*
 * VUnescape() unescapes escaped character.
 *
 * @param      in                 [inout] pointer to string.
 * @return     0                  if success.
 */
	int
_VUnescape( char *in )
{
	int			i;
	int			index;
	int			len;
	char		c1, c2;

	//DLOG(  "_VUnescape() enter..\n");

	len = strlen(in);

	for ( i = 0, index = 0; i < len; i++ )
	{
		c1 = in[i];

		if ( c1 == '\\' )
		{
			c2 = in[i+1];
			if ( c2 == ';' ) continue;
		}
		in[index++] = c1;
	}

	in[index] = '\0';

	return(0);
}

/*
 * VEscape() escapes character.
 *
 * @param      in                 [inout] pointer to string.
 * @return     0                  if success.
 */
	int
_VEscape( char *in )
{
	int			i;
	int			index;
	int			len;
	char *buf = NULL;
	char		c;

	//DLOG(  "_VEscape() enter..\n");

	len = strlen(in);
	buf = (char*) calloc(1, len*2+1);

	for ( i = 0, index = 0; i < len; i++ ){
		c = in[i];
		if ( c == ';' )	{
			buf[index++] = '\\';
		}
		buf[index++] = c;
	}

	strncpy( in, buf, len*2+1 );
	free(buf);

	return(0);
}



/*
 * vManySpace2Space() converts multiple spaces to single space in 'in'.
 *
 * @param      in                 [inout] pointer to string.
 * @return     int                length of converted string.
 */
	int
_VManySpace2Space( char *in )
{
	int		i, j;
	int		spaced = 0;

	//DLOG(  "_VManySpace2Space() enter..\n");

	j = 0;
	for ( i = 0; in[i]; i++ )
	{
		if ( _VIsSpace( in[i] ) )
		{
			if ( !spaced )
			{
				in[j] = WSP;
				spaced = 1;
				j++;
			}
		}
		else
		{
			spaced = 0;
			in[j] = in[i];
			j++;
		}
	}


	in[j] = '\0';

	return j;
}


/**
 * vFindBase64() returns the integer repesentation of the location in base64 table.
 *
 * @param      in                a character
 * @return     int               The base64 table location of input character
 */
static int __VFindBase64( char in )
{
	int		i;

	//DLOG(  "__VFindBase64() enter..\n");

	for ( i = 0; i < 65; i++ )
	{
		if ( Base64Table[i] == in )
			return i;
	}
	return -1;
}



/**
 * vBase64Check() returns the total length of input except non-base64 value.
 *
 * @param      in               char values which are base64 or non-base64
 * @return     int              the total length of input except non-base64
 */
static int __VBase64Check( char *in )
{
	int		i = 0, j = 0;
	int		base;

	//DLOG(  "__VBase64Check() enter..\n");

	while ( in[i] )
	{
		base = __VFindBase64( in[i] );
		if ( base < 0 )
		{
			i++;
		}
		else
		{
			in[j] = in[i];
			j++; i++;
		}
	}

	in[j] = '\0';

	return j;
}

/**
 * vBase64Decoder() decodes the base64 encoded input.
 *
 * @param      Src                Base64 encoded input
 * @param      Dest               The destination buffer of decoded value
 * @return     int                The total length decoded value
 */
	int
_VB64Decode( char *Dest, char *Src )
{
	char*		Encoded = Src;
	int			i, j = 0;
	int			res;
	char		Base = 0;
	char		DecodeTemp;
	char		Debuffer[4] = {0x00, 0x00, 0x00, '\0'};
	int			index = 0;
	int			len;

	//DLOG(  "_VB64Decode() enter..\n");

	len = __VBase64Check( Src );

	while ( *Encoded )
	{
		for ( i = 0; i < 3; i++ )
			Debuffer[i] = 0x00;

		for ( i = 0; i < 4; i++, Encoded++, j++ )
		{
			if(*Encoded == 0x00) break;
			if((res = __VFindBase64(*Encoded)) < 0)	continue;

			Base = ( char )res;
			DecodeTemp = 0x00;

			if(Base == 64)	{
				Encoded++;
				break;
			}

			switch ( i )
			{
			case 0:
				DecodeTemp = Base << 2;
				Debuffer[0] |= DecodeTemp;
				break;
			case 1:
				DecodeTemp = Base >> 4;
				Debuffer[0] |= DecodeTemp;
				DecodeTemp = Base << 4;
				Debuffer[1] |= DecodeTemp;
				break;
			case 2:
				DecodeTemp = Base >> 2;
				Debuffer[1] |= DecodeTemp;
				DecodeTemp = Base << 6;
				Debuffer[2] |= DecodeTemp;
				break;
			case 3:
				DecodeTemp = Base;
				Debuffer[2] |= DecodeTemp;
				break;
			}
		}

		if ( Base == 64 )
		{
			switch ( i )
			{
			case 0: break;
			case 1:
			case 2:
				Dest[index] = Debuffer[0];
				index++;
				break;
			case 3:
				Dest[index++] = Debuffer[0];
				Dest[index++] = Debuffer[1];
				break;
			}
		}
		else
		{
			Dest[index++] = Debuffer[0];
			Dest[index++] = Debuffer[1];
			Dest[index++] = Debuffer[2];
		}
	}

	return index;
}


/**
 * vBase64Encoder() encode the input to base64.
 *
 * @param      Src                non-base64 char input
 * @param      Dest               The destination buffer of encoded value
 * @return     0
 */
int _VB64Encode( char *Dest, char *Src, int len )
{
	char*	Encoded = Dest;
	char*	Decoded = Src;
	int	i, j;
	int	index;
	int	res = 0;
	int	base1 = 0, base2 = 0;
	char	Enbuffer[4] = {0};
	char	Debuffer[3] = {0};
	int	length = 0;

	//DLOG(  "_VB64Encode() enter..\n");

	for ( i = 0; i < len; i++ )
	{
		res = i%3;

		switch ( res )
		{
		case 0:
			Debuffer[0] = *Decoded;
			break;
		case 1:
			Debuffer[1] = *Decoded;
			break;
		case 2:
			Debuffer[2] = *Decoded;
			index = ( int )( ( Debuffer[0] & 0xFC ) >> 2 );
			Enbuffer[0] = Base64Table[index];
			base1 = ( int )( ( Debuffer[0] & 0x03 ) << 4 );
			base2 = ( int )( ( Debuffer[1] & 0xF0 ) >> 4 );
			index = ( int )( base1 | base2 );
			Enbuffer[1] = Base64Table[index];
			base1 = ( int )( ( Debuffer[1] & 0x0F ) << 2 );
			base2 = ( int )( ( Debuffer[2] & 0xC0 ) >> 6 );
			index = ( int )( base1 | base2 );
			Enbuffer[2] = Base64Table[index];
			index = ( int )( Debuffer[2] & 0x3F );
			Enbuffer[3] = Base64Table[index];

			Encoded[length++] = Enbuffer[0];
			Encoded[length++] = Enbuffer[1];
			Encoded[length++] = Enbuffer[2];
			Encoded[length++] = Enbuffer[3];

			for ( j = 0; j < 3; j++ )
				Debuffer[j] = 0x00;

			break;
		}

		Decoded++;
	}

	res = i % 3;

	switch ( res )
	{
	case 0:
		break;
	case 1:
		index = ( int )( ( Debuffer[0] & 0xFC ) >> 2 );
		Enbuffer[0] = Base64Table[index];
		base1 = ( int )( ( Debuffer[0] & 0x03 ) << 4 );
		base2 = ( int )( ( Debuffer[1] & 0xF0 ) >> 4 );
		index = ( int )( base1 | base2 );
		Enbuffer[1] = Base64Table[index];
		Enbuffer[2] = Base64Table[64];
		Enbuffer[3] = Base64Table[64];

		Encoded[length++] = Enbuffer[0];
		Encoded[length++] = Enbuffer[1];
		Encoded[length++] = Enbuffer[2];
		Encoded[length++] = Enbuffer[3];

		break;
	case 2:
		index = ( int )( ( Debuffer[0] & 0xFC ) >> 2 );
		Enbuffer[0] = Base64Table[index];
		base1 = ( int )( ( Debuffer[0] & 0x03 ) << 4 );
		base2 = ( int )( ( Debuffer[1] & 0xF0 ) >> 4 );
		index = ( int )( base1 | base2 );
		Enbuffer[1] = Base64Table[index];
		base1 = ( int )( ( Debuffer[1] & 0x0F ) << 2 );
		base2 = ( int )( ( Debuffer[2] & 0xC0 ) >> 6 );
		index = ( int )( base1 | base2 );
		Enbuffer[2] = Base64Table[index];
		Enbuffer[3] = Base64Table[64];

		Encoded[length++] = Enbuffer[0];
		Encoded[length++] = Enbuffer[1];
		Encoded[length++] = Enbuffer[2];
		Encoded[length++] = Enbuffer[3];

		break;
	}

	Encoded[length] = '\0';

	return 0;
}


/**
 * vUnfolding() unfold the folded line.
 *
 * @param      string             The folded line input
 * @return     int                Total length of unfolded output
 */
int _VUnfolding( char *string )
{
	unsigned int i, j;
	unsigned int len;

	//DLOG(  "_VUnfolding() enter..\n");

	len = strlen( string );

	for ( i = 0, j = 0; i < len; i++, j++ )
	{
		string[j] = string[i];

		// 12.03.2004 Process garbage character at the end of vcard/vcal
		if ( _VIsSpace( string[i] ) && ( i < len-5 ) )
		{
			if ( string[i-1] == LF || string[i-1] == CR )
			{
				j -= 2; string[i-1] = 0;
			}
			if ( string[i-2] == LF || string[i-2] == CR )
			{
				j -= 1; string[i-2] = 0;
			}
		}
	}

	string[j] = '\0';

	return j;
}


int __VIsNewTypeforOrg( char *pCardRaw, int vType )
{
	int count=0, i = 0, low=0, high=0, diff=0,vTypeNum;
	char strTypeName[50]={0};
	extern const char *pszCalTypeList[];
	//SysDebug(( MID_VDATA, "__VIsNewType() enter..\n"));

	while(1)
	{
		if(*pCardRaw == CR || *pCardRaw == LF)
			pCardRaw++;
		else
		{
			if(*pCardRaw == ';' || *pCardRaw == ':' || count >= 50)
			{
				break;
			}
			else
				strTypeName[count++] = *pCardRaw++;
		}
	}

	if(vType == VCALENDAR)
		vTypeNum = VCAL_TYPE_NUM;
	else
		return false;

	for ( low = 0, high = vTypeNum - 1; high >= low; diff < 0 ? ( low = i+1 ) : ( high = i-1 ) )
	{
		i = ( low + high ) / 2;

		if(vType == VCALENDAR)
			diff = strcmp( pszCalTypeList[i], strTypeName );

		if ( diff == 0 ) 	 /* success: found it */
			return true;
		else if( !strncmp( strTypeName, "X-", 2 )) /* jpds-835 X-NEC-SUMMARY, X-NEC-FILENAME, X-NO, X-CLASS 등 특정 폰에서 지원하는 TYPE인 경우에 대한 처리 추가 by sohn */
			return true;
	}

	//if(count <= 50) return TRUE;

	return false;

	//res = __VCardGetName( strTypeName, (char**)pszCardTypeList, VCARD_TYPE_NUM );
}


/**
 * vUnfolding() unfold the folded line.
 *
 * @param      string             The folded line input
 * @return     int                Total length of unfolded output
 */
	int
_VUnfoldingNoSpec( char *string, int vType )
{
	unsigned int i, j;
	unsigned int len;

	//SysDebug(( MID_VDATA, "_VUnfolding() enter..\n" ));

	len = strlen( string );

	for ( i = 0, j = 0; i < len; i++, j++ )
	{
		string[j] = string[i];

		if ( ( i < len-5 ) )
		{
			if ( string[i] == '=' )
			{
				if(string[i+1] == CR && string[i+2] == LF && string[i+3] =='=' )  // quoted printable이고, 다음줄로 string이 연결되는 경우
				{
					string[i] = 0;
					string[i+1] = 0;
					string[i+2] = 0;
					j -= 1;
					i += 2;
				}
				//				else if(string[i+1] == CR || string[i+1] == LF)  // CR 이나 LF 둘중에 한개만 존재하는 경우
				else if(string[i+1] == CR && string[i+2] == LF && __VIsNewTypeforOrg(&string[i+3], vType) == false)  //string이 연결되는 경우
				{
					string[i] = 0;
					string[i+1] = 0;
					string[i+2] = 0;
					j -= 1;
					i += 2;
				}
			}
			else if(string[i] ==WSP
					||string[i]==TAB)
			{
				if(string[i-2] == CR && string[i-1] == LF)
				{
					string[i] = 0;
					string[i-1] = 0;
					string[i-2] = 0;
					j -= 3;
				}
				else if(string[i-1] == CR || string[i-1] == LF)	 // CR 이나 LF 둘중에 한개만 존재하는 경우
				{
					string[i] = 0;
					string[i-1] = 0;
					j -= 2;
				}
			}

		}
	}

	string[j] = '\0';

	return j;
}

/**
 * vFolding() decodes the base64 encoded input.
 *
 * @param      contentline        Original line (unfolded)
 * @param      Dest               The destination buffer of folded result
 */
	void
_VFolding( char *result, char *contentline )
{
	int		i = 0;

	//DLOG(  "_VFolding() enter..\n");

	while ( *contentline )
	{
		if ( i == 75 )
		{
			i = 0;
			*result++ = '\r';
			*result++ = '\n';
			*result++ = ' ';
		}

		*result++ = *contentline++;
		i++;
	}

	*result++ = '\0';
}


/**
 * vFolding() decodes the base64 encoded input.
 *
 * @param      contentline        Original line (unfolded)
 * @param      Dest               The destination buffer of folded result
 */
	void
_VFoldingQP( char *result, char *contentline )
{
	int		i = 0;

	//DLOG(  "_VFolding() enter..\n");

	while ( *contentline )
	{
		if ( i == 74 )
		{
			i = 0;
			*result++= '=';
			*result++ = '\r';
			*result++ = '\n';
		}

		*result++ = *contentline++;
		i++;
	}

	*result++ = '\0';
}


/**
 * vFolding() decodes the base64 encoded input.
 *
 * @param      contentline        Original line (unfolded)
 * @param      Dest               The destination buffer of folded result
 */
	void
_VFoldingNoSpace( char *result, char *contentline )
{
	int		i = 0;

	//DLOG(  "_VFolding() enter..\n");

	while ( *contentline )
	{
		if ( i == 75 )
		{
			i = 0;
			*result++ = '\r';
			*result++ = '\n';
		}

		*result++ = *contentline++;
		i++;
	}

	*result++ = '\0';
}


/**
 * vQuotedPrintalbeDecoder() decodes the quoted-printable encoded input.
 *
 * @param      Src                Quoted-printable encoded input
 * @return     int                The total length decoded value
 */
	int
_VQPDecode( char *src )
{
	int		i = 0, j = 0;
	char	qp[2];
	char	decodedNum;

	//DLOG(  "_VQPDecode() enter..\n");

	while ( src[i] )
	{
		if ( src[i] == '=' )
		{
			if ( !( _VIsSpace( src[i + 1] ) || ( src[i + 1] == '\r' ) || ( src[i+1] == '\n' ) ) )
			{
				if ( src[i + 1] == '0' && ( src[i + 2] == 'D' || src[i +2] == 'd' ) && src[i + 3] == '='
						&& src[i + 4] == '0' && ( src[i + 5] == 'A' || src[i + 5] == 'a' ) )
				{
					src[j] = '\n';
					j++;
					i += 6;
				}
				else
				{
					qp[0] = src[i + 1];
					qp[1] = src[i + 2];
					decodedNum = __VHexaDecoder( qp );
					src[j] = decodedNum;
					i += 3; j++;
				}
			}
			else
			{
				i += 3;
			}
		}
		else
		{
			src[j] = src[i];
			i++; j++;
		}
	}

	src[j] = '\0';

	j =	_VManySpace2Space( src );

	return j;
}



/**
 * vQuotedPrintableEncoder() decodes the quoted-printalbe encoded input.
 *
 * @param      Src                Quoted-printable encoded input
 * @param      Dest               The destination buffer of decoded value
 * @return     int                The total length decoded value
 */
	int
_VQPEncode( char *dest, char *src )
{
	int		i = 0, j = 0, k = 0;
	char	encoded[2] = {0x0f, 0x0f};

	//DLOG(  "_VQPEncode() enter..\n");

	while ( src[i] /*&& ( src[i] > 0 )*/ )
	{
		if ( k == 73 && _VIsSpace( src[i] ) )
		{
			if( src[i] == WSP )
			{
				dest[j++] = '='; dest[j++] = '2'; dest[j++] = '0';
				k += 3;
			}
			else if ( src[i] == TAB )
			{
				dest[j++] = '='; dest[j++] = '0'; dest[j++] = '9';
				k += 3;
			}
		}
		/*	else if ( k == 76 )
			{
			dest[j++] = '='; dest[j++] = WSP;
			k = 0;
			} */
		else if ( !__VIsPrintable( src[i] ) )
		{
			dest[j++] = '=';
			encoded[0] &= (src[i] >> 4);
			encoded[1] &= (src[i]);
			__VHexaEncoder( encoded );
			dest[j++] = encoded[0]; encoded[0] = 0x0f;
			dest[j++] = encoded[1]; encoded[1] = 0x0f;
			k += 3;
		}
		else if ( src[i] == '\r' || src[i] == '\n' )
		{
			dest[j++] = '='; dest[j++] = '0'; dest[j++] = 'D'; k += 3;
			dest[j++] = '='; dest[j++] = '0'; dest[j++] = 'A'; k += 3;
		}
		else
		{
			dest[j++] = src[i]; k++;
		}
		i++;
	}

	dest[j] = '\0';

	return j;
}


/**
 * vIsPrintable() check whether the input is printable.
 *
 * @param      in
 * @return     true/false            if input is printable :true else : false
 */
static int __VIsPrintable( char in )
{
	//DLOG(  "__VIsPrintable() enter..\n");

	if ( in >= 33 && in <= 60 ) return true;
	else if ( in >= 62 && in <= 126 ) return true;
	else if ( in == WSP || in == TAB ) return true;
	else if ( in == '\r' || in == '\n' ) return true;
	else return false;
}



/**
 * vHexaDecoder() output the character value of inputed hexadecimal value.
 *
 * @param      qp               Hexadecimal input value
 * @return     char             Character representation of input hexadecimal value
 */
static char __VHexaDecoder( char *qp )
{
	int		i;
	char	decoded[2] = {0x00, 0x00};
	char	res;

	//DLOG(  "__VHexaDecoder() enter..\n");

	for ( i = 0; i < 2; i++ )
	{
		switch ( qp[i] )
		{
		case '0':
			decoded[i] = 0x00;
			break;
		case '1':
			decoded[i] = 0x01;
			break;
		case '2':
			decoded[i] = 0x02;
			break;
		case '3':
			decoded[i] = 0x03;
			break;
		case '4':
			decoded[i] = 0x04;
			break;
		case '5':
			decoded[i] = 0x05;
			break;
		case '6':
			decoded[i] = 0x06;
			break;
		case '7':
			decoded[i] = 0x07;
			break;
		case '8':
			decoded[i] = 0x08;
			break;
		case '9':
			decoded[i] = 0x09;
			break;
		case 'a':
		case 'A':
			decoded[i] = 0x0a;
			break;
		case 'b':
		case 'B':
			decoded[i] = 0x0b;
			break;
		case 'c':
		case 'C':
			decoded[i] = 0x0c;
			break;
		case 'd':
		case 'D':
			decoded[i] = 0x0d;
			break;
		case 'e':
		case 'E':
			decoded[i] = 0x0e;
			break;
		case 'f':
		case 'F':
			decoded[i] = 0x0f;
			break;
		}
	}

	res = ( char )( ( decoded[0] << 4 ) + decoded[1] );

	return res;
}



/**
 * vHexaEncoder() output the hexadecimal value of input character value.
 *
 * @return     qp               Character representation of input hexadecimal value
 */
static void __VHexaEncoder( char *qp )
{
	int		i;

	//DLOG(  "__VHexaEncoder() enter..\n");

	for ( i = 0; i < 2; i++ )
	{
		switch ( qp[i] )
		{
		case 0:
			qp[i] = '0';
			break;
		case 1:
			qp[i] = '1';
			break;
		case 2:
			qp[i] = '2';
			break;
		case 3:
			qp[i] = '3';
			break;
		case 4:
			qp[i] = '4';
			break;
		case 5:
			qp[i] = '5';
			break;
		case 6:
			qp[i] = '6';
			break;
		case 7:
			qp[i] = '7';
			break;
		case 8:
			qp[i] = '8';
			break;
		case 9:
			qp[i] = '9';
			break;
		case 10:
			qp[i] = 'A';
			break;
		case 11:
			qp[i] = 'B';
			break;
		case 12:
			qp[i] = 'C';
			break;
		case 13:
			qp[i] = 'D';
			break;
		case 14:
			qp[i] = 'E';
			break;
		case 15:
			qp[i] = 'F';
			break;
		}
	}

}

/**
 * _VIsCrLf() returns one if char is either a space, tab, or newline.
 *
 * @param      s1                [in] pointer to first string.
 * @param      s2                [in] pointer to second string.
 * @return     1                 'in' is a space character.
 * @return     0                 'in' is not a space.
 */
	int
_VIsCrLf(char in)
{
	//SysDebug(( MID_VDATA, "_VIsCrLf() enter..\n" ));

	if ( ( in == CR ) || ( in == LF ) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 * vManySpace2Space() converts multiple spaces to single space in 'in'.
 *
 * @param      in                 [inout] pointer to string.
 * @return     int                length of converted string.
 */
	int
_VManyCRLF2CRLF(char *pIn)
{
	int		i, j;
	bool	bCrLf = false, bFirstCrLf = true;

	//SysDebug(( MID_VDATA, "_VManySpace2Space() enter..\n" ));

	j = 0;
	for ( i = 0; pIn[i]; i++ )
	{
		if ( _VIsCrLf( pIn[i] ) && _VIsCrLf( pIn[i+1] ))
		{
			if( bFirstCrLf && !bCrLf)
			{
				bCrLf = 1;
			}
			else if( !bFirstCrLf )
			{
				if ( !bCrLf )
				{
					pIn[j] = CR;
					pIn[++j] = LF;
					bCrLf = true;
					j++;
				}
			}
			i++;
		}
		else
		{
			bCrLf = false;
			bFirstCrLf = false;
			pIn[j] = pIn[i];
			j++;
		}
	}

	pIn[j] = '\0';

	return j;
}


























