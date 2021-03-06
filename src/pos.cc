#include <sys/types.h>
#include <stdio.h>
#include "define.h"
#include "struct.h"


/*
 *   LOCAL FUNCTIONS
 */


bool  can_wake     ( char_data* );
bool  sit_trigger  ( char_data*, obj_data* );
bool  sit          ( char_data*, obj_data* );


/*
 *   CONSTANTS
 */


const char* position_name [] = { "dead", "mortally wounded",
  "incapacitated", "stunned", "sleeping", "meditating", "resting",
  "fighting", "standing", "flying" };


void update_pos( char_data* ch )
{
  if( ch->hit > 0 ) {
    if( ch->position < POS_SLEEPING ) {
      ch->position = POS_RESTING;
      renter_combat( ch );
      }
    return;
    }

       if( ch->hit <= -11 ) ch->position = POS_DEAD;
  else if( ch->hit <= -6 )  ch->position = POS_MORTAL;
  else if( ch->hit <= -3 )  ch->position = POS_INCAP;
  else                      ch->position = POS_STUNNED; 

  ch->fighting = NULL;
  disrupt_spell( ch );
}

inline obj_clss_data* get_obj_index( int );

/*
 *   CHAIR ROUTINES
 */


bool sit( char_data* ch, obj_data* obj )
{
  int         people = 0;
  char_data*    rch;
  
  for( int i = 0; i < *obj->array; i++ )
    if( ( rch = character( obj->array->list[i] ) ) != NULL )
      if( rch->pos_obj == obj )
        if( same_plane( ch, rch ) )
          people++;
  
  if(  people >= obj->pIndexData->value[0]  )
  {
    send( ch, "There are too many people on that.\n\r" );
    return FALSE;
  }
 
 ch->pos_obj     = obj;
 
 return TRUE;
};


void unseat( char_data* ch )
{
  if( ch->pos_obj == NULL )
  {
      bug( "Unseat a non-sitting character.\n\r" );
      return;
  }
  
  ch->pos_obj =  NULL;

  return;
}


/*
 *   MEDITATE
 */ 


void do_meditate( char_data* ch, char* argument )
{
  if( is_mob( ch ) )
    return;

  if( ch->species == NULL && ch->get_skill( SKILL_MEDITATE ) == UNLEARNT )
  {
    send( ch, "You do not have the knowledge required to bring yourself into\
 a state of\n\rmeditation.\n\r" );
    return;
  }

  if( ch->mount != NULL )
  {
    send( ch, "You can't meditate while mounted.\n\r" );
    return;
  }

  if( water_logged( ch->in_room ) )
  {
    send( ch, "You can't meditate underwater.\n\r" );
    return;
  }

  if( ch->fighting != NULL ) {
    send( ch,
      "Meditating while fighting requires more concentration than you\
 can muster.\n\r");
    return;
  }

  if( ch->position == POS_MEDITATING ) {
    send( ch, "You are already meditating.\n\r" );
    return;
  }
  
  if( ch->position == POS_STANDING )
  {
    if( *argument != '\0' )
    {
      obj_data* sit_obj;

      if( !strncmp( "on ", argument, 3 ) )
        for( int i = 3; argument[i] != '\0'; i++ )
          if( !isspace( argument[i] ) )
          {
            argument += 3;
            break;
          }
  
      if( ( sit_obj = one_object( ch, argument, "meditate on", ch->array ) ) == NULL ) 
        return;
      
      if( !is_set( sit_obj->pIndexData->extra_flags, OFLAG_RUG ) )
      {
        if( is_set( sit_obj->pIndexData->extra_flags, OFLAG_CHAIR ) )
          send( ch, "%s is too uncomfortable for meditating.\n\r", sit_obj->Name( ) );
        else
          send( ch, "You can't meditate on %s.\n\r", sit_obj->Name( ) );
        return;
      }
      
      if( ch->pos_obj == sit_obj || sit( ch, sit_obj ) )
      {
        send( ch, "You sit down on %s and start to meditate.\n\r", sit_obj->Name( ) );
        fsend_seen( ch, "%s sits down on %s and starts to meditate.", ch, sit_obj->Name( ) );
        ch->position = POS_MEDITATING;
        sit_trigger( ch, sit_obj );
      }
      return;
    }
    
    if( ch->pos_obj != NULL )
    {
      if( is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_RUG ) )
      {
        send( ch, "You sit down and start to meditate on %s.\n\r", ch->pos_obj->Name( ) );
        fsend_seen( ch, "%s sits down and starts to meditate on %s.", ch, ch->pos_obj->Name( ) );
        ch->position = POS_MEDITATING;
        return;
      }
      else
        unseat( ch );
    }

    send( "You sit down and start to meditate.\n\r", ch );
    send_seen( ch, "%s sits down and starts to meditate.\n\r", ch );
  }
  else
  {
    if( ch->pos_obj != NULL
      && !is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_RUG ) )
    {
      fsend( ch, "%s you're on is too uncomfortable for meditating.",
        Obj_the_Name( ch->pos_obj ) );
      return;
    }
    send( "You start to meditate.\n\r", ch );
    send_seen( ch, "%s starts to meditate.\n\r", ch );
  }
  
  ch->position = POS_MEDITATING;
  return;
}


/*
 *   LIE DOWN
 */


void do_lie( char_data* ch, char* )
{
  do_sleep( ch, "" );

  return;
}


/*
 *   SIT
 */


void do_sit( char_data* ch, char* argument )
{

  obj_data*      obj;
  thing_data*  thing;
  char_data*     sch;
  int              i;

  if( *argument == '\0' )
  {
    do_rest( ch, "" );
    return;
  }
  
  if( opponent( ch ) != NULL ) {
    send( ch, "You can't sit down while fighting!" );
    return;
  }

  if( ch->position != POS_STANDING )
  {
    send( ch,
      "You need to be standing before you can sit on an object.\n\r" );
      return;
  }
  
  if( !strncmp( "on ", argument, 3 ) )
    for( i = 3; argument[i] != '\0'; i++ )
      if( !isspace( argument[i] ) )
      {
        argument += 3;
        break;
      }

  if( ( obj = one_object( ch, argument,
    "sit on", ch->array ) ) == NULL ) 
    return;
   
  if( !is_set( obj->pIndexData->extra_flags, OFLAG_CHAIR )
    && !is_set( obj->pIndexData->extra_flags, OFLAG_RUG ) )
  { 
    send( "That can't be sat on.\n\r", ch );
    return;
  }
    
  if( obj == ch->pos_obj || sit( ch, obj ) )
  {
    if( is_set( obj->pIndexData->extra_flags, OFLAG_RUG ) )
    {
      send( ch, "You rest on %s.\n\r", obj->Name( ) );
      fsend_seen( ch, "%s rests on %s.", ch, obj->Name( ) );
    }
    else
    {
      send( ch, "You sit on %s.\n\r", obj->Name( ) );
      fsend_seen( ch, "%s sits on %s.", ch, obj->Name( ) );
    }
    ch->position = POS_RESTING;
    sit_trigger( ch, obj );
  }
 
  return;
}


bool sit_trigger( char_data *ch, obj_data *obj )
{
  oprog_data* oprog;

  for( oprog = obj->pIndexData->oprog; oprog != NULL; oprog = oprog->next )
    if( oprog->trigger == OPROG_TRIGGER_SIT ) {
      var_ch   = ch;
      var_obj  = obj; 
      var_room = ch->in_room;
      execute( oprog );
      return TRUE;
      }
  
  return FALSE;
}


void do_rest( char_data* ch, char*  )
{
  if( ch->mount != NULL ) {
    send( "You need to dismount first.\n\r", ch );
    return;
    }

  if( deep_water( ch->in_room ) ) {
    send( ch, "You can't rest while swimming.\n\r" );
    return;
    }

  if( ch->fighting != NULL ) {
    send( ch, "Rest while fighting??\n\r" );
    return;
    }

  switch( ch->position ) {
    case POS_SLEEPING:
      if( can_wake( ch ) ) {
        send( ch, "You wake and sit up.\n\r" );
        send_seen( ch, "%s wakes and sits up.\n\r", ch );
        ch->position = POS_RESTING;
        }
      break;

    case POS_RESTING:
      send( ch,"You are already resting.\n\r" );
      break;

    case POS_MEDITATING:
      send( ch, "You stop meditating and sit up.\n\r" );
      fsend_seen( ch, "%s stops meditating and begins resting.", ch );
      ch->position = POS_RESTING;
      break;

    case POS_STANDING:
      if( ch->rider != NULL ) {
        send( ch, "You lie down, dismounting %s.\n\r", ch->rider );
        send( ch->rider, "Your mount lies down, dismounting you.\n\r" );
        send_seen( ch, "%s lies down, dismounting %s.\n\r", ch, ch->rider );
        dismount( ch->rider );
        }
      else {
        if( ch->pos_obj != NULL )
        {
          if( is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_RUG ) )
          {
            send( ch, "You sit down on %s.\n\r", ch->pos_obj->Name( ) );
            fsend_seen( ch, "%s sits down on %s.", ch, ch->pos_obj->Name( ) );
          }
          else
          {
            send( ch, "You hop off %s and sit down on the floor.\n\r", ch->pos_obj->Name( ) );
            fsend_seen( ch, "%s hops off %s and sits down on the floor.", ch, ch->pos_obj->Name( ) );
            unseat( ch );
          }
        }
        else
        {
          send( ch, "You sit down on the floor.\n\r" );
          send_seen( ch, "%s sits down on the floor.\n\r", ch );
        }
      }
      ch->position = POS_RESTING;
      break;
    }

  return;
}


/*
 *   SLEEP 
 */


bool can_wake( char_data* ch )
{
  if( !is_set( ch->affected_by, AFF_SLEEP ) || is_immortal( ch ) ) 
    return TRUE;
  
  send( ch, "You are magically slept and unable to wake.\n\r" );

  return FALSE;
} 


void do_sleep( char_data* ch, char* argument )
{
  char_data* keeper;

  if( ch->mount != NULL ) {
    send( ch, "You can't sleep while riding.\n\r" );
    return;
    }

  if( water_logged( ch->in_room ) ) { 
    send( ch, "You can't sleep underwater.\n\r" );
    return;
    } 

  if( ( keeper = active_shop( ch ) ) != NULL ) {
    send( ch, "%s won't allow vagrants to sleep in %s shop.\n\r", 
      keeper, keeper->His_Her( ) );
    return;
    }

  if( ch->fighting != NULL ) {
    send( ch, "It tends to be difficult to fall asleep while fighting.\n\r" );
    return;
    }
  
  if( *argument != '\0' && ch->position >= POS_STANDING )
  {
    obj_data* sit_obj;
    
    if( !strncmp( "on ", argument, 3 ) )
      for( int i = 3; argument[i] != '\0'; i++ )
        if( !isspace( argument[i] ) )
        {
          argument += 3;
          break;
        }

    if( ( sit_obj = one_object( ch, argument, "sleep on", ch->array ) ) == NULL ) 
      return;
   
    if( !is_set( sit_obj->pIndexData->extra_flags, OFLAG_CHAIR )
      && !is_set( sit_obj->pIndexData->extra_flags, OFLAG_RUG ) )
    { 
      send( ch, "You can't sleep on %s.\n\r", sit_obj->Name( ) );
      return;
    }
    
    if( sit( ch, sit_obj ) )
    {
      if( is_set( sit_obj->pIndexData->extra_flags, OFLAG_RUG ) )
      {
        send( ch, "You lie down on %s and go to sleep.\n\r", sit_obj->Name( ) );
        fsend_seen( ch, "%s lies down on %s and goes to sleep.", ch, sit_obj->Name( ) );
      }
      else
      {
        send( ch, "You sit down on %s and go to sleep.\n\r", sit_obj->Name( ) );
        fsend_seen( ch, "%s sits down on %s and goes to sleep.", ch, sit_obj->Name( ) );
      }
      sleep( ch );
      sit_trigger( ch, sit_obj );
    }
    return;
  }

  switch( ch->position ) {
    case POS_SLEEPING:
      send( ch, "You are already asleep.\n\r" );
      break;

    case POS_MEDITATING:
      sleep( ch );
      send( ch, "You stop meditating and go to sleep.\n\r");
        send_seen( ch, "%s goes to sleep.\n\r", ch );
      break;
    case POS_RESTING:
      sleep( ch );
      send( ch, "You sleep.\n\r" );
      if( ch->pos_obj != NULL
        && !is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_RUG ) )
        fsend_seen( ch, "%s goes to sleep on %s.", ch, ch->pos_obj->Name( ) );
      else
        send_seen( ch, "%s lies down and goes to sleep.\n\r", ch );

      break;
    case POS_STANDING: 
      sleep( ch );
      if( ch->pos_obj != NULL )
      {
        if( is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_RUG ) )
        {
          send( ch, "You lie down on %s and go to sleep.\n\r", ch->pos_obj->Name( ) );
          fsend_seen( ch, "%s lies down on %s and goes to sleep.", ch, ch->pos_obj->Name( ) );
        }
        else
        {
          send( ch, "You sleep on %s.\n\r", ch->pos_obj->Name( ) );
          fsend_seen( ch, "%s goes to sleep on %s.", ch, ch->pos_obj->Name( ) );
        }
      }
      else
      {
        send( ch, "You sleep.\n\r" );
        send_seen( ch, "%s lies down and goes to sleep.\n\r", ch );
      }
      break;
    }

  return;
}


void sleep( char_data *ch )
{
  disrupt_spell( ch );
  ch->presleep = ch->position; /* Smith - 3-5-2000 - old state storage */
  ch->position = POS_SLEEPING;

  if( ch->rider != NULL ) {
    send( ch->rider,
      "As %s drops to sleep, you quickly find yourself dismounted.\n\r", ch );
    fsend_seen( ch->rider,
      "As %s falls to sleep, %s quickly finds %sself dismounted.",
      ch->rider, ch->rider->He_She( ), ch->rider->Him_Her( ) );
    ch->rider->mount = NULL;
    ch->rider = NULL;
    }

  return;
}


/*
 *   STAND 
 */


void do_stand( char_data* ch, char* argument )
{
  if( ch->mount != NULL ) {
    send( ch, "Use dismount to stand on the ground.\n\r" );
    return;
    }

  if( *argument != '\0' && ( ch->position == POS_STANDING
    || ch->position == POS_RESTING ) )
  {
    obj_data* sit_obj;
    
    if( !strncmp( "on ", argument, 3 ) )
      for( int i = 3; argument[i] != '\0'; i++ )
        if( !isspace( argument[i] ) )
        {
          argument += 3;
          break;
        }

    if( ( sit_obj = one_object( ch, argument, "stand on", ch->array ) ) == NULL ) 
      return;
    
    if( ch->position == POS_RESTING )
      if( sit_obj != ch->pos_obj )
      {
        send( ch, "You need to be standing before you can stand on an object.\n\r" );
        return;
      }
      else
        unseat( ch );
    
    if( ch->position == POS_STANDING && sit_obj == ch->pos_obj )
    {
      send( ch, "You're already standing on that.\n\r" );
      return;
    }
       
    if( !is_set( sit_obj->pIndexData->extra_flags, OFLAG_CHAIR )
     && !is_set( sit_obj->pIndexData->extra_flags, OFLAG_RUG ) )
    { 
      send( ch, "You can't stand on that.\n\r", sit_obj->Name( ) );
        return;
    }
   
    if( sit( ch, sit_obj ) )
    {
      if( ch->position == POS_RESTING )
      {
        send( ch, "You stand up on %s.\n\r", sit_obj->Name( ) );
        fsend_seen( ch, "%s stands up on %s.", ch, sit_obj->Name( ) );
        ch->position = POS_STANDING;
      }
      else
      {
        if( is_set( sit_obj->pIndexData->extra_flags, OFLAG_CHAIR ) )
        {
          send( ch, "You stand up on %s.\n\r", sit_obj->Name( ) );
          fsend_seen( ch, "%s stands up on %s.", ch, sit_obj->Name( ) );
        }
        else
        {
          send( ch, "You stand on %s.\n\r", sit_obj->Name( ) );
          fsend_seen( ch, "%s stands on %s.", ch, sit_obj->Name( ) );
        }
      }
      sit_trigger( ch, sit_obj );
    }
    return;
  }

  switch ( ch->position ) {
    case POS_SLEEPING:
      if( can_wake( ch ) ) { 
        if( ch->pos_obj != NULL
          && is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_CHAIR ) )
        {
          if( is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_RUG ) )
          {
            send( ch, "You wake and clumber off %s.\n\r", Obj_the_Name( ch->pos_obj ) );
            fsend_seen( ch, "%s wakes and clumbers off %s.", ch, Obj_the_Name( ch->pos_obj ) );
            unseat( ch );
            ch->position = POS_STANDING;
            break;
          }
          unseat( ch );
        }
        send( ch, "You wake and stand up.\n\r" );
        send_seen( ch, "%s wakes and stands up.\n\r", ch );
        ch->position = POS_STANDING;
      }
      break;

    case POS_MEDITATING:
      if( ch->pos_obj != NULL
          && is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_CHAIR ) )
        {
          send( ch, "You stop meditating and hop off %s.\n\r", Obj_the_Name( ch->pos_obj ) );
          fsend_seen( ch, "%s stops meditating and hops off %s.",
          ch, Obj_the_Name( ch->pos_obj ) );
          unseat( ch );
        }
        else
        {
          send( "You stop meditating and stand up.\n\r", ch );
          send_seen( ch, "%s stops meditating and stands up.\n\r", ch );
        }
      ch->position = POS_STANDING;
      break;

    case POS_RESTING:
      if( ch->pos_obj != NULL
        && is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_CHAIR ) ) {
        send( ch, "You get off %s and stand up.\n\r", ch->pos_obj->Name( ) );
        fsend_seen( ch, "%s gets off %s and stands up.", ch, ch->pos_obj->Name( ) );
        unseat( ch );
      }
      else {
        if( opponent( ch ) != NULL ) {
          send( ch, "You're trying dern it.\n\r" );
          return;
	    }
        else { 
          send( ch, "You stand up.\n\r" );
          send_seen( ch, "%s stands up.\n\r", ch );
	    }
      }
      ch->position = POS_STANDING;
      break;

    case POS_STANDING:
      if( ch->pos_obj != NULL )
      {
        if( is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_CHAIR )
          && is_set( ch->pos_obj->pIndexData->extra_flags, OFLAG_RUG ) )
        {
          send( ch, "You hop off %s.\n\r", ch->pos_obj->Name( ) );
          fsend_seen( ch, "%s hops off %s.", ch, ch->pos_obj->Name( ) );
        }
        else
        {
          send( ch, "You get off %s.\n\r", ch->pos_obj->Name( ) );
          fsend_seen( ch, "%s gets off %s.", ch, ch->pos_obj->Name( ) );
        }
        unseat( ch );
      }
      else
        send( ch, "You are already standing.\n\r" );
      break;
    }

  return;
}


/*
 *   WAKE
 */


void do_wake( char_data* ch, char* argument )
{
  char_data* victim;

  if( *argument == '\0' ) {
    if( ch->position > POS_SLEEPING ) {
      send( ch, "You aren't sleeping.\n\r" );
      return;
      }
/* Smith - 3-5-2000 - ADDED return to old state */
    switch (ch->presleep) {
      case POS_STANDING:
        do_stand( ch, "" );
        break;
      case POS_RESTING:
        do_rest( ch, "" );
        break;
      case POS_MEDITATING:
        do_meditate( ch, "" );
        break; 
    }
    return;
  }

  if( ch->position <= POS_SLEEPING ) {
    send( ch, "You are asleep yourself!\n\r" );
    return;
    }

  if( ( victim = one_character( ch, argument, "wake",  ch->array ) ) == NULL )
    return;

  if( victim->position > POS_SLEEPING ) {
    send( ch, "%s is already awake.\n\r", victim );
    return;
    }

  if( victim->position < POS_SLEEPING || IS_AFFECTED( victim, AFF_SLEEP ) ) {
    send( ch, "You try to wake %s, but %s doesn't react.\n\r",
      victim, victim->He_She( ) );
    return;
    }

  victim->position = POS_RESTING;

  send( ch, "You wake %s.\n\r", victim->Him_Her( ) );
  send( victim, "%s wakes you.\n\r", ch );
  send_seen( ch, "%s wakes %s.\n\r", ch, victim );
}






