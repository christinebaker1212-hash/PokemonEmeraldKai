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
