#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"


void put_in         ( char_data*, thing_array*, obj_data* );
void put_obj_on     ( char_data*, char_data*, thing_array* );
void put_obj_on     ( char_data*, obj_data*,  obj_data* );
void wear_obj       ( char_data*, obj_data*, char_data* );


/*
 *   DO_PUT ROUTINE
 */


void do_put( char_data* ch, char* argument )
{
  char               arg  [ MAX_STRING_LENGTH ];
  thing_array*     array;
  obj_data*    container;
  char_data*      victim;


  if( contains_word( argument, "on", arg ) ) {
    if( (
      victim = one_character( ch, argument, "put on", ch->array ) ) != NULL )
      if( ( array = several_things( ch, arg, "put", &ch->contents ) ) != NULL )
        put_obj_on( ch, victim, array );
      return;
    }

  if( !two_argument( argument, "in", arg ) ) {
    send( ch, "Syntax: put <obj> [in] <container>.\n\r" );
    return;
    }

  if( ( container = one_object( ch, argument, "put in",
    &ch->wearing, &ch->contents, ch->array ) ) == NULL )
    return;

  /* PIPES */

  /*  
  if( container->pIndexData->item_type == ITEM_PIPE ) {
    if( obj->selected != 1 || obj->next_list != NULL ) {
      send( ch, "You can only put one item in a pipe at a time.\n\r" );
      }
    else if( container->contents != NULL ) {
      send( ch, "%s already contains %s.\n\r",
        container, container->contents );
      }
    else if( obj->pIndexData->item_type != ITEM_TOBACCO ) {
      send( ch, "%s is not something you wish to smoke.\n\r", obj );
      }
    else {
      obj = remove( obj, obj->selected );
      put_obj( obj, container );
      send( ch, "You put %s into %s.\n\r", obj, container );
      send_seen( ch, "%s puts %s into %s.\n\r", ch, obj, container );
      }
    return;
    }    
  */

  /* BAGS & KEYRINGS */

  if( container->pIndexData->item_type != ITEM_CONTAINER
    && container->pIndexData->item_type != ITEM_KEYRING ) {
    fsend( ch,
      "You can only put items in containers and keyrings and %s is neither.",
      container );
    return;
    }

  if( is_set( &container->value[1], CONT_CLOSED) ) {
    send( ch, "%s is closed.\n\r", container );
    return;
    }

  if( ( array = several_things( ch, arg, "put",
    &ch->contents ) ) == NULL )
    return;

  put_in( ch, array, container );
}


/*
 *   BAGS 
 */


thing_data* holding( thing_data* thing, char_data*, thing_data* )
{
  obj_data* obj = (obj_data*) thing;
  
  if( obj->pIndexData->item_type == ITEM_CONTAINER 
    && is_set( &obj->value[1], CONT_HOLDING ) )
    return NULL;

  if( !is_empty( obj->contents ) )
    for( int i = 0; i < obj->contents; i++ )
      if( holding( obj->contents[i], NULL, NULL ) == NULL )
        return NULL;

  return thing;
}


thing_data* wont_fit( thing_data* thing, char_data*, thing_data* container )
{
  int  n  = thing->Weight( thing->number );
  int  m  = container->Capacity( );
  
  if( m >= n || n <= 0 )
    return thing;
  
  if( ( m = thing->selected*m/n ) <= 0 )
    return NULL;

  content_array* where = thing->array;

  thing = thing->From( m );
  thing->To( where );

  return thing;
}


thing_data* put( thing_data* thing, char_data* ch, thing_data* container )
{
  thing = thing->From( thing->selected );

  if( ch != carried_by( container ) )
    set_owner( (obj_data*) thing, NULL, ch );

  thing->To( container );

  return thing;
}


void put_in( char_data* ch, thing_array* array, obj_data* container )
{
  thing_array    subset  [ 5 ];
  thing_func*      func  [ 5 ]  = { same, cursed, wont_fit, holding, put };
  content_array*  where;

  where = container->array;
  container = (obj_data*) container->From( 1 );

  sort_objects( ch, *array, container, 5, subset, func );
  
  msg_type = MSG_INVENTORY;

  page_priv( ch, NULL, empty_string );
  page_priv( ch, &subset[0], "can't fold", NULL, "into itself" );
  page_priv( ch, &subset[1], "can't let go of" );
  page_priv( ch, &subset[2], "doesn't have room for", container );
  page_priv( ch, &subset[3], "mysteriously can't get", container, "into" );
  page_publ( ch, &subset[4], "put", container, "into",
    container->array == ch->array ? " on the ground" : empty_string );

  consolidate( subset[4] );
  container->To( where );
  consolidate( container );

  delete array;
}


/*
 *   HORSES
 */

thing_data*   cantwear         ( thing_data*, char_data*, thing_data* );
thing_data*   levellimit       ( thing_data*, char_data*, thing_data* );
void          list_horse_wear  ( char_data*, thing_array*, char_data* );

void put_on( char_data* ch, thing_array* array, char_data* victim )
{
  char              tmp  [ ONE_LINE ];
  char             tmp2  [ ONE_LINE ];
  thing_array      list;
  thing_data*     thing;
  thing_array    subset  [ 5 ];
  thing_func*      func  [ 5 ]  = { cantwear, cursed, levellimit,
                                    needremove, NULL };
  
  for( int i = array->size-1; i >= 0; i-- )
    if( stolen( thing = array->list[i], ch, NULL ) == NULL ) {
        list += thing;
        *array -= thing;
      }
  
  sort_objects( victim, *array, (thing_data*) &subset[4], 5, subset, func );

  msg_type = MSG_EQUIP;

  page_priv( ch, NULL, empty_string );
  page_priv( ch, &list, NULL, NULL, "is stolen", "are stolen" );
  sprintf( tmp, "on %s", victim->Name( ch ) );
  page_priv( ch, &subset[0], "you can't put", NULL, tmp );
  sprintf( tmp, "isn't something %s can cope with", victim->Name( ch ) );
  sprintf( tmp2, "aren't something %s can cope with", victim->Name( ch ) );
  page_priv( ch, &subset[1], "can't let go of" );
  page_priv( ch, &subset[2], NULL, NULL, tmp, tmp2 );
  page_priv( ch, &subset[3], "need to remove something to put on" );
  list_horse_wear( ch, &subset[4], victim );
  
  delete array;
}

void put_obj_on( char_data* ch, char_data* victim, thing_array* array )
{
  if( victim->leader != ch || !is_set( &victim->status, STAT_PET ) ) {
    send( ch, "%s isn't a pet of yours.\n\r", victim );
    delete array;
    return;
    }
  
  if( ch->mount == victim ) {
    send( ch, "You need to dismount first.\n\r" );
    delete array;
    return;
    }
  
  if( victim->rider != NULL ) {
    send( ch, "%s is mounted by someone.\n\r", victim );
    delete array;
    return;
    }
  
  put_on( ch, array, victim );
}

/* PUIOK 5/6/2000 - changed mind, delete if more then 2 weeks old
void wear_obj( char_data* victim, obj_data* obj, char_data* ch )
{
  int   loc  = -1;

  if( is_set( &victim->species->wear_part, WEAR_HORSE_BACK )
    && can_wear( obj, ITEM_WEAR_HORSE_BACK ) ) {
    if( victim->Wearing( WEAR_HORSE_BACK ) != NULL ) {
      send( ch, "%s already has something on %s back.\n\r",
        victim, victim->His_Her( ) );
      return;
      }
    loc = WEAR_HORSE_BACK;
    send( ch, "You place %s on the back of %s.\n\r", obj, victim );
    send_seen( ch, "%s places %s on the back of %s.\n\r", ch, obj, victim ); 
    }

  else if( is_set( &victim->species->wear_part, WEAR_HORSE_BODY )
    && can_wear( obj, ITEM_WEAR_HORSE_BODY ) ) {
    if( victim->Wearing( WEAR_HORSE_BODY ) != NULL ) {
      send( ch, "%s is already wearing something to protect %s body.\n\r",
        victim, victim->His_Her( ) );
      return;
      }
    loc = WEAR_HORSE_BODY;
    send( ch, "You put %s on %s, protecting %s body.\n\r",
      obj, victim, victim->His_Her( ) );
    send_seen( ch, "%s puts %s on %s, protecting %s body.\n\r",
      ch, obj, victim, victim->His_Her( ) ); 
    }

  else if( is_set( &victim->species->wear_part, WEAR_HORSE_FEET )
    && can_wear( obj, ITEM_WEAR_HORSE_FEET ) ) {
    if( victim->Wearing( WEAR_HORSE_FEET ) != NULL ) {
      send( ch, "%s already has something on %s feet.\n\r", 
        victim, victim->His_Her( ) );
      return;
      }
    loc = WEAR_HORSE_FEET;
    send( ch, "You place %s on the feet of %s.\n\r", obj, victim );
    send_seen( ch, "%s puts %s on the feet of %s.\n\r", ch, obj, victim ); 
    }

  else {
    send( ch, "You can't put %s on %s.\n\r", obj, victim );  
    return;
    }

  obj = (obj_data*) obj->From( 1 );
  obj->position = loc;
  obj->layer    = 0;
  obj->To( &victim->wearing );
}
*/

/*
 *   TABLES
 */


void put_obj_on( char_data* ch, obj_data* container, list_data* list )
{
  /*
  obj_data*   obj;
  int           i;

  if( container->pIndexData->item_type != ITEM_TABLE ) {
    send( "You can only place items on tables.\n\r", ch );
    return;
    }

  for( i = 0; i < list->length; i++ ) {
    obj = (obj_data*) list->pntr[i];
    if( obj->array != ch )
      continue;
    if( container->where != ch->in_room && container->where != ch )
      break;
    if( !obj->droppable( ) ) {
      send( ch, "You can't let go of %s.\n\r", obj );
      continue;
      }
    act_to( "You place $p on $P.", ch, obj, container, ch );
    act_notchar( "$n places $p on $P.", ch, obj, container );
    obj = remove( obj, obj->selected );
    put_obj( obj, container );
    }
  
  */
  return;
}

