"""SINNOH's gyms that move (Platinum's src/overlay008/gym_features.c): the models Platinum loads at run time into
each gym (not in its blocks' props), by gym map. build_pack.py packs each one with its gym's textures (the pack's
named meshes, version 4); gen_gyms.py gives the game their model numbers (src/data/sinnoh_gyms.h). "fx:" names are
Platinum's field effect models (res/graphics/field_effects: their own textures), numbered FX_BASE + their place in
fx_names()."""
FX_BASE = 0x8000

GYM_MODELS = {
    'MAP_HEADER_PASTORIA_CITY_GYM': ['pastoria_gym_water_floor.nsbmd'],
    'MAP_HEADER_CANALAVE_CITY_GYM': ['canalave_gym_second_floor.nsbmd', 'canalave_gym_third_floor.nsbmd',
                                     'canalave_gym_fourth_floor.nsbmd', 'canalave_gym_platform_red.nsbmd',
                                     'canalave_gym_platform_x_direction.nsbmd', 'canalave_gym_platform_y_direction.nsbmd',
                                     'canalave_gym_platform_z_direction.nsbmd'],
    'MAP_HEADER_SUNYSHORE_CITY_GYM_ROOM_1': ['sunyshore_gear_L.nsbmd', 'sunyshore_gear_L_alt.nsbmd',
                                             'sunyshore_gear_T.nsbmd', 'sunyshore_gear_T_alt.nsbmd',
                                             'sunyshore_gear_dead_end.nsbmd', 'sunyshore_gear_vertical.nsbmd',
                                             'sunyshore_gear_vertical_alt.nsbmd'],
    'MAP_HEADER_ETERNA_CITY_GYM': ['eterna_gym_model_clock_hour_hand.nsbmd', 'eterna_gym_model_clock_minute_hand.nsbmd'],
    'MAP_HEADER_VEILSTONE_CITY_GYM': ['fx:veilstone_gym_sandbag.nsbmd', 'fx:veilstone_gym_tire_stack.nsbmd'],
    'MAP_HEADER_SNOWPOINT_CITY_GYM': ['fx:snowpoint_gym_snowball.nsbmd'],
    'MAP_HEADER_HEARTHOME_CITY_GYM_TRAINER_ROOM_1': ['fx:hearthome_gym_hint_%s.nsbmd' % n for n in (
        'circle', 'square', 'triangle', 'sun', 'target', 'moon', 'star', 'heart')],   # (the clue: Platinum's door order)
    # not a gym's: IRON ISLAND B2F's platform lift, loaded at run time (src/sinnoh/s3d_lift.c; the others are their map's)
    'MAP_HEADER_IRON_ISLAND_B2F_LEFT_ROOM': ['iron_island_platform_lift.nsbmd'],
    # not a gym's: the GREAT MARSH's tram (src/sinnoh/s3d_tram.c)
    'MAP_HEADER_GREAT_MARSH_1': ['great_marsh_train.nsbmd'],
    # not a gym's: the VILLA's furniture, shown once bought (Platinum's villa_furniture.c; src/sinnoh/s3d_villa.c)
    'MAP_HEADER_VILLA': ['villa_furniture_%s.nsbmd' % n for n in (
        'table', 'big_sofa', 'small_sofa', 'bed', 'night_table', 'tv', 'audio_system', 'bookshelf', 'rack', 'houseplant',
        'pc_desk', 'music_box', 'pokemon_bust', 'pokemon_bust_silver', 'piano', 'guest_set', 'wall_clock', 'masterpiece',
        'tea_set', 'chandelier')],
    # not a gym's: the healing machine's balls in every POKeMON CENTER (src/sinnoh/s3d_heal.c; the CENTERs share textures)
    'MAP_HEADER_JUBILIFE_CITY_POKECENTER_1F': ['pokecenter_healing_machine_mini_pokeball.nsbmd'],
    # not a gym's: the DISTORTION WORLD's platforms, elevators and props (overlay009's sProp3DModelNARCIndexByKind
    # order; src/sinnoh/s3d_distortion.c, numbers through tools/sinnoh/gen_distortion.py)
    'MAP_HEADER_DISTORTION_WORLD_B1F': ['fx:distortion_world_%s.nsbmd' % n for n in (
        'small_platform', 'floating_blue_rock', 'medium_elevator_platform_1', 'medium_elevator_platform_2',
        'medium_elevator_platform_sw', 'medium_moving_platform_1', 'medium_moving_platform_2',
        'medium_elevator_platform_3', 'large_elevator_platform_1', 'medium_elevator_platform_4',
        'horizontal_platform_1', 'vertical_platform_1', 'vertical_platform_2', 'horizontal_platform_2',
        'rotated_platform_east', 'rotated_platform_west', 'large_elevator_platform_2', 'large_elevator_platform_3',
        'large_elevator_platform_4', 'medium_elevator_platform_5', 'giratina_shadow', 'waterfall', 'land_vine_flower',
        'land_rock', 'portal')],
    # not a gym's: SINNOH's signposts, signboards and mailboxes (Platinum's objects with these models; src/sinnoh/s3d_signs.c,
    # where they stand from tools/sinnoh/gen_signs.py)
    'MAP_HEADER_TWINLEAF_TOWN': ['fx:%s.nsbmd' % n for n in (
        'map_signpost', 'arrow_signpost', 'gym_signpost', 'signboard', 'trainer_tips', 'mailbox')],
}


def model_names():
    """Every model above, once, in order"""
    out = []
    for names in GYM_MODELS.values():
        for n in names:
            if n not in out:
                out.append(n)
    return out


def fx_names():
    return [n for n in model_names() if n.startswith('fx:')]


def number(src, name):
    """The model number the pack and the game know NAME by"""
    if name.startswith('fx:'):
        return FX_BASE + fx_names().index(name)
    return src._props.index(name)
