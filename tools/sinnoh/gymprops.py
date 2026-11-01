"""SINNOH's gyms that move (Platinum's src/overlay008/gym_features.c): the models Platinum loads at run time into
each gym (not in its blocks' props), by gym map. build_pack.py packs each one with its gym's textures (the pack's
named meshes, version 4); gen_gyms.py gives the game their model numbers (src/data/sinnoh_gyms.h)."""

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
}


def model_names():
    """Every model above, once, in order"""
    out = []
    for names in GYM_MODELS.values():
        for n in names:
            if n not in out:
                out.append(n)
    return out
