#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"


/*
 *   LOCAL FUNCTIONS
 */


obj_data*  has_recall      ( char_data* );


/*
 *   SET WIMPY FUNCTION
 */


const char* wimpy_info = "--------------------\n\r\\                   \\\n\r/    Wimpy Values   /\n\r\\                   \\\n\r/     Flee : %3d    /\n\r\\    Blink : %3d    \\\n\r/     Pray : %3d    /\n\r\\   Recall : %3d    \\\n\r/                   /\n\r--------------------\n\r";






const char* wimpy_word [] = { "flee", "blink", "pray", "recall" };


void do_wimpy( char_data* ch, char* argument )
{
  int*   value;
  int      num;

  if( is_mob( ch ) )
    return;

  value = unpack_int( ch->pcdata->wimpy );

  if( *argument == '\0' ) {
    send( ch, wimpy_info, value[0], value[1], value[2], value[3] );
    return;
    }

  for( int i = 0; i < 4; i++ ) {
    if( matches( argument, wimpy_word[i] ) ) {
      if( ( num = atoi( argument ) ) < 0 || num > 255 ) {
        send( ch, "You can only set wimpy values from 0 to 255.\n\r" );
        return;
        }
      value[i]          = num;
      ch->pcdata->wimpy = pack_int( value );
      send( ch, "Your wimpy setting for %s set to %d.\n\r", 
        wimpy_word[i], num );
      return;
      }
    }

  send( ch, "Syntax: wimpy <field> <value>\n\r" );
}


/*
 *   AUTO-FLEE HANDLER
 */


bool check_wimpy( char_data* ch )
{
  cast_data*   prepare;
  int*           value;

  if( opponent( ch ) == NULL ) 
    return FALSE;

  if( ch->species != NULL ) {
    if( ch->hit < ch->max_hit/4
	&& is_set( &ch->species->act_flags, ACT_WIMPY )
	&& number_range( 0,5 ) == 0 ) 
      return attempt_flee( ch );
    return FALSE;
  }
  
  if( is_set( &ch->status, STAT_BERSERK )
      || !is_set( &ch->status, STAT_WIMPY ) ) 
    return FALSE;
  
  value = unpack_int( ch->pcdata->wimpy );

  if( ch->hit <= value[3] && ( has_recall( ch ) != NULL ) ) {
    do_recite( ch, "recall" ); 
     return TRUE;
    }
  else if( ch->hit <= value[2] ) {
    do_pray( ch, "" );
    return TRUE;    
    }
  else if( ch->hit <= value[1] 
    && ch->shdata->skill[ SPELL_BLINK ] != 0 
    && ( prepare = has_prepared( ch, SPELL_BLINK-SPELL_FIRST ) ) != NULL ) {
    do_cast( ch, "blink" );
    }
  else if( ch->hit <= value[0] ) {
    attempt_flee( ch );
    }
}


obj_data* has_recall( char_data* ch )
{
  obj_data* obj;

  for( int i = 0; i < ch->contents; i++ ) {
    if( ( obj = object( ch->contents[i] ) ) != NULL 
      && obj->pIndexData->vnum == OBJ_RECALL &&
	is_set( obj->extra_flags, OFLAG_IDENTIFIED ) )
      return obj;
    }

  return NULL;
}


/*
 *   FLEE ROUTINE
 */


void do_flee( char_data* ch, char* argument )
{
  room_data* room;
  exit_data* exit  = NULL;

  if( has_enemy( ch ) == NULL ) {
    send( ch, "Noone is attacking you.\n\r" );
    return;
    }

  if( is_set( &ch->status, STAT_BERSERK ) ) {
    send( ch,
      "You can never flee while the battle lust rages within you!\n\r" );
    return;
    }

  if( is_set( ch->affected_by, AFF_ENTANGLED ) ) {
    send( ch, "You can't flee while entangled!\n\r" );
    return;
    }

  if( ( room = Room( ch->array->where ) ) == NULL ) {
    send( ch, "You aren't in a room which is confusing.\n\r" );
    return;
    }

  if( *argument != '\0' ) {
    if( ( exit = (exit_data*) one_thing( ch, argument, "flee",
      (thing_array*) &room->exits ) ) == NULL )
      return;
    if( !ch->Can_Move( exit ) ) {
      send( ch, "You can't flee %s.\n\r", dir_table[ exit->direction ].name );
      return;
      }
    }

  attempt_flee( ch, exit );
}


bool attempt_flee( char_data* ch, exit_data* exit )
{
  content_array*  was_in;
  content_array*  now_in;
  int                exp;

  if( exit == NULL
    && ( exit = random_movable_exit( ch ) ) == NULL )
    return FALSE;

  if( number_range( 0, 100 ) < 10+20*ch->get_burden() ) {
    send( ch, "You attempt to flee, but fail to escape the battle!\n\r" );
    set_delay( ch, 32 );
    return TRUE;
    }

  was_in = ch->array;
  move_char( ch, exit->direction, TRUE );

  if( !ch->Is_Valid( ) || ch->hit <= 0 )
    return TRUE;

  if( ch->array == was_in ) {
    send( ch,
      "You flee but for some reason end up where you started!!\n\r" );
    return TRUE;
    }

  now_in    = ch->array;
  ch->array = was_in;

  send_seen( ch, "%s flees %s!\n\r",
    ch, dir_table[ exit->direction ].name );

  ch->array = now_in;

  if( ch->pcdata != NULL && ch->species == NULL ) {
    exp = (ch->shdata->level*ch->hit)/2;
    if( ch->shdata->level == 1 )
      exp = min( ch->exp, exp );
    if( exp > 0 ) 
      add_exp( ch, -exp, "You lose %d exp for fleeing.\n\r" );
    }

  set_delay( ch, 32 );
  clear( ch->cmd_queue );

  return TRUE;
}










