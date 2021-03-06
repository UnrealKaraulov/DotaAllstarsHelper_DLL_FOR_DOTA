#include "Main.h"




#define MAX_CHAT_MSG_LEN 128

int GetChatOffset( )
{
	int pclass = *( int* )pW3XGlobalClass;
	if ( pclass > 0 )
	{
		return *( int* )( pclass + 0x3FC );
	}

	return 0;
}




char * GetChatString( )
{
	int pChatOffset = GetChatOffset( );
	if ( pChatOffset > 0 )
	{
		pChatOffset = *( int* )( pChatOffset + 0x1E0 );
		if ( pChatOffset > 0 )
		{
			pChatOffset = *( int* )( pChatOffset + 0x1E4 );
			return ( char * )pChatOffset;
		}
	}
	return 0;
}

LPARAM lpReturnScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_RETURN, 0 ) << 16 ) );
LPARAM lpReturnScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_RETURN, 0 ) << 16 ) );



LPARAM lpRShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_RSHIFT, 0 ) << 16 ) );
LPARAM lpRShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_RSHIFT, 0 ) << 16 ) );

LPARAM lpShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_SHIFT, 0 ) << 16 ) );
LPARAM lpShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_SHIFT, 0 ) << 16 ) );

LPARAM lpLShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_LSHIFT, 0 ) << 16 ) );
LPARAM lpLShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_LSHIFT, 0 ) << 16 ) );






pGameChatSetState GameChatSetState;


typedef int( __fastcall * pGameChatSendMessage )( int GlobalGlueObjAddr, int zero, int event_vtable );
pGameChatSendMessage GameChatSendMessage;

pSetChatTargetUsers pSetChatTargetUsers_org;
pSetChatTargetUsers pSetChatTargetUsers_ptr;

BOOL UsingCustomChatTarget = FALSE;

int CustomChatTarget = 0;

int __fastcall SetChatTargetUsers_my( int chataddr, int ecx, int valtype )
{
	if ( !UsingCustomChatTarget )
		return pSetChatTargetUsers_ptr( chataddr, ecx, valtype );
	else
		return pSetChatTargetUsers_ptr( chataddr, ecx, CustomChatTarget );
}


time_t AntiSpam_LastTime = std::time( 0 );
unsigned int AntiSpam_Seconds = 4;
unsigned int AntiSpam_CurMsgCount = 0;
unsigned int AntiSpam_MsgLimit = 10;


void __stdcall SetAntiSpamLimits( unsigned int Messages, unsigned int Seconds )
{
	AntiSpam_Seconds = Seconds;
	AntiSpam_MsgLimit = Messages;
}

int _EventVtable = 0;
int _ChatSendEvent = 0;

int __stdcall SendMessageToChat( const char * msg, BOOL toAll )
{
	/* Send messages to chat without any delay!  */
	/* by Karaulov 08.12.2017 */


	if ( !msg || msg[ 0 ] == '\0' )
		return FALSE;
	
	if ( AntiSpam_MsgLimit && AntiSpam_Seconds )
	{
		time_t AntiSpam_CurTime = std::time( 0 );
		if ( AntiSpam_CurTime - AntiSpam_LastTime > AntiSpam_Seconds )
		{
			AntiSpam_LastTime = AntiSpam_CurTime;
			AntiSpam_CurMsgCount = 0;
		}
		else
		{
			AntiSpam_CurMsgCount++;

			if ( AntiSpam_CurMsgCount > AntiSpam_MsgLimit )
			{
				return FALSE;
			}
		}
	}

	int ChatOffset = GetChatOffset( );
	if ( !ChatOffset )
	{
		return FALSE;
	}

	char * pChatString = GetChatString( );

	if ( !pChatString )
	{
		return FALSE;
	}

	//unsigned char tmpbuf[ 256 ];
	//unsigned char tmpbuf2[ 256 ];
	//memset( tmpbuf2, 0, 256 );
	//GetKeyboardState( tmpbuf );
	//SetKeyboardState( tmpbuf2 );

	
	//char tmpdeb[ 512 ];
	//sprintf_s( tmpdeb, "%X->%s", pChatString, pChatString );
	//MessageBox( 0, tmpdeb, tmpdeb, 0 );

	BlockInput( TRUE );

	if ( pChatString > 0 && Warcraft3Window > 0 )
	{
		if ( *( int* )ChatFound )
		{
			UsingCustomChatTarget = TRUE;
			if ( toAll )
			{
				CustomChatTarget = 0;
			}
			else
			{
				CustomChatTarget = 1;
			}
			/* Close chat */
			pChatString[ 0 ] = '\0';
			GameChatSetState( ChatOffset, 0, 0 );

			/* Open chat */
			GameChatSetState( ChatOffset, 0, 1 );

			/* Set message */
			sprintf_s( pChatString, MAX_CHAT_MSG_LEN, "%.128s", msg );

			/* Send Event */
			GameChatSendMessage = ( pGameChatSendMessage )( _ChatSendEvent );
			GameChatSendMessage( *( int* )_GlobalGlueObj, 0, _EventVtable );

			UsingCustomChatTarget = FALSE;
		}
		else
		{
			UsingCustomChatTarget = TRUE;

			if ( toAll )
			{
				CustomChatTarget = 0;
			}
			else
			{
				CustomChatTarget = 1;
			}

			/* Open chat */
			GameChatSetState( ChatOffset, 0, 1 );

			/* Set message */
			sprintf_s( pChatString, MAX_CHAT_MSG_LEN, "%.128s", msg );

			/* Send Event */
			GameChatSendMessage = ( pGameChatSendMessage )( _ChatSendEvent );
			GameChatSendMessage( *( int* )_GlobalGlueObj, 0, _EventVtable );

			UsingCustomChatTarget = FALSE;

		}

	}
	BlockInput( FALSE );
 //	SetKeyboardState( tmpbuf );

	return 0;
}