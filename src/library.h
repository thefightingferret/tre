/*
 *   VARIABLES
 */


extern obj_data*   var_container;
extern char_data*  var_victim;  
extern char_data*  var_mob; 
extern char_data*  var_ch;
extern obj_data*   var_obj;
extern room_data*  var_room;
extern char*       var_arg;
extern char*       var_str;
extern int         var_i;
extern int         var_j;
extern char_data*  var_rch;


/*
 *   OPERATORS
 */


cfunc code_and;
cfunc code_eor;
cfunc code_or;
cfunc code_minus_equal;
cfunc code_plus_equal;
cfunc code_set_equal;
cfunc code_not_equal;
cfunc code_is_equal;
cfunc code_gt;
cfunc code_lt;
cfunc code_le;
cfunc code_ge;


/*
 *   SCRIPT FUNCTIONS
 */


cfunc code_acode;
cfunc code_act_area;
cfunc code_act_bark;
cfunc code_act_notchar;
cfunc code_act_notvict;
cfunc code_act_room;
cfunc code_act_neither;
cfunc code_act_tochar;
cfunc code_add_path;
cfunc code_atoi;
cfunc code_assign_quest;
cfunc code_attack;
cfunc code_attack_acid;
cfunc code_attack_cold;
cfunc code_attack_fire;
cfunc code_attack_shock;
cfunc code_attack_room;
cfunc code_attack_weapon;
cfunc code_can_fly;
cfunc code_can_move;
cfunc code_cast_spell;
cfunc code_cat_string;
cfunc code_cflag;
cfunc code_char_in_room;
cfunc code_class;
cfunc code_close;
cfunc code_coin_value;
cfunc code_cond_act;
cfunc code_check_cond;
cfunc code_dam_message;
cfunc code_dice;
cfunc code_disarm;
cfunc code_do_spell;
cfunc code_doing_quest;
cfunc code_done_quest;
cfunc code_drain_stat;
cfunc code_drain_exp;
cfunc code_find_char;
cfunc code_find_hp;
cfunc code_find_maxhp;
cfunc code_find_maxmp;
cfunc code_find_maxmv;
cfunc code_find_mob;
//Added Trinity 4-9-00
cfunc code_find_mount;
//End Addition
cfunc code_find_mp;
cfunc code_find_player;
cfunc code_find_room;
cfunc code_find_skill;
cfunc code_find_stat;
cfunc code_find_vnum;
cfunc code_get_leader;  /* PUIOK 1/6/2000 */
cfunc code_get_name;
cfunc code_get_room;
cfunc code_get_time;
cfunc code_get_victim;
cfunc code_get_vnum_room;
cfunc code_has_obj;
/* code_mflag - zemus - april 14 */
cfunc code_has_mflag;
cfunc code_has_quest;
cfunc code_heal;
cfunc code_inflict;
cfunc code_inflict_acid;
cfunc code_inflict_cold;
cfunc code_inflict_fire;
cfunc code_inflict_shock;
cfunc code_int_string;
cfunc code_interpret;
cfunc code_is_exhausted;
cfunc code_is_locked;
cfunc code_is_fighting;
cfunc code_is_follower;
cfunc code_is_mounted;
cfunc code_is_name;
cfunc code_is_objtype;
cfunc code_is_open;
cfunc code_is_player;
cfunc code_is_playername;
cfunc code_is_resting;
cfunc code_is_searching;
cfunc code_is_silenced;
cfunc code_is_time_range;
cfunc code_junk_mob;
cfunc code_junk_obj;
cfunc code_lock;
cfunc code_look;
cfunc code_mpcode;
cfunc code_mload;
cfunc code_mob_in_room;
cfunc code_modify_mana;
cfunc code_modify_move; 
cfunc code_num_in_room;
cfunc code_num_mob;
cfunc code_oload;
cfunc code_obj_name;
cfunc code_obj_in_room;
cfunc code_obj_to_char;
cfunc code_obj_to_room;
cfunc code_obj_value;
cfunc code_ofind;
cfunc code_opcode;
cfunc code_open;
cfunc code_plague;
cfunc code_players_area;
cfunc code_players_room;
cfunc code_poison;
cfunc code_race;
cfunc code_raise_exp;
cfunc code_random;
cfunc code_rand_char;
cfunc code_rand_player;
cfunc code_remove_cflag;
cfunc code_remove_rflag;
cfunc code_reputation;
cfunc code_rflag;
cfunc code_sacrifice;
cfunc code_send_to_area;
cfunc code_send_to_char;
cfunc code_send_to_room;
cfunc code_set_cflag;
cfunc code_set_hp;
cfunc code_set_mp;
cfunc code_set_mv;
cfunc code_set_religion;
cfunc code_set_rflag;
cfunc code_show;
cfunc code_show_room;
cfunc code_size;
cfunc code_summon;
cfunc code_tell;
cfunc code_transfer;
cfunc code_transfer_all;
cfunc code_unlock;
cfunc code_update_quest;
cfunc code_wait;
cfunc code_with_coins;
cfunc code_weight;

