@ Due to the way mks4agb (Nintendo's tool) works, key split table labels can
@ appear before the actual start of the key split table data. If you look at
@ the first keysplit table (keysplit_piano), you'll notice it's offset backwards
@ by 36 bytes. This is because the key split tables don't map instruments
@ for the entire note range (0-127)--they only map subsets, and the upper
@ and lower ranges aren't necessarily 0 or 127.
@
@ For example if a key split table maps an note range of 10-20, then the key
@ split table label will be offset 10 bytes before the actual key split data
@ begins. Therefore, the notes naturally map to the key split table without
@ any extra offset calculation.

keysplit piano, 36
	split 0, 55
	split 1, 70
	split 2, 91
	split 3, 108

keysplit strings, 36
	split 0, 69
	split 1, 81
	split 2, 108

keysplit trumpet, 36
	split 0, 66
	split 1, 84
	split 2, 108

keysplit tuba, 24
	split 0, 42
	split 1, 108

keysplit french_horn, 36
	split 0, 66
	split 1, 108

@ Moemon Kai: Johto music (tools/quetzal_port/gen_music.py)

keysplit qz_610_tubular_bells, 55
	split 0, 67
	split 1, 79
	split 2, 82

keysplit qz_610_percussive_organ, 53
	split 0, 60
	split 1, 78
	split 2, 90
	split 3, 97

keysplit qz_610_clean_guitar, 48
	split 1, 60
	split 2, 78

keysplit qz_610_acoustic_bass, 33
	split 0, 35
	split 1, 48
	split 2, 51

keysplit qz_610_string_ensemble_1, 48
	split 1, 54
	split 2, 70
	split 3, 75

keysplit qz_610_trumpet, 48
	split 0, 65
	split 1, 85
	split 2, 87

keysplit qz_610_trombone, 53
	split 1, 63
	split 2, 70

keysplit qz_610_french_horn, 47
	split 0, 56
	split 1, 109

keysplit qz_songs_acoustic_grand_piano, 26
	split 0, 44
	split 1, 56
	split 2, 68
	split 3, 80
	split 4, 108

keysplit qz_songs_bright_piano, 37
	split 0, 68
	split 1, 94

keysplit qz_songs_electric_grand_piano, 31
	split 0, 44
	split 1, 56
	split 2, 68
	split 3, 82

keysplit qz_songs_harpsichord, 26
	split 0, 68
	split 1, 80
	split 2, 112

keysplit qz_songs_clavinet, 32
	split 0, 66
	split 1, 75
	split 2, 94

keysplit qz_songs_celesta, 51
	split 0, 72
	split 1, 93

keysplit qz_songs_vibraphone, 29
	split 0, 56
	split 1, 68
	split 2, 98

keysplit qz_songs_tubular_bells, 33
	split 0, 53
	split 1, 61
	split 2, 70
	split 3, 99

keysplit qz_songs_drawbar_organ, 28
	split 0, 48
	split 1, 61
	split 2, 73
	split 3, 90

keysplit qz_songs_church_organ, 28
	split 0, 46
	split 1, 67

keysplit qz_songs_reed_organ, 34
	split 0, 54
	split 1, 78

keysplit qz_songs_harmonica, 26
	split 0, 44
	split 1, 66

keysplit qz_songs_tango_accordion, 30
	split 0, 66
	split 1, 75
	split 2, 84
	split 3, 95

keysplit qz_songs_nylon_guitar, 52
	split 0, 70
	split 1, 78
	split 2, 79

keysplit qz_songs_steel_guitar, 24
	split 0, 49
	split 1, 58
	split 2, 88

keysplit qz_songs_jazz_guitar, 40
	split 0, 49
	split 1, 58
	split 2, 66
	split 3, 75
	split 4, 87

keysplit qz_songs_guitar_harmonics, 31
	split 0, 55

keysplit qz_songs_acoustic_bass, 43
	split 0, 54
	split 1, 63
	split 2, 85

keysplit qz_songs_fingered_bass, 49
	split 0, 70
	split 1, 77
	split 2, 97

keysplit qz_songs_picked_bass, 41
	split 0, 61
	split 1, 70
	split 2, 91

keysplit qz_songs_fretless_bass, 50
	split 0, 66
	split 1, 75
	split 2, 82
	split 3, 106

keysplit qz_songs_slap_bass_1, 30
	split 0, 46
	split 1, 54
	split 2, 87

keysplit qz_songs_slap_bass_2, 67
	split 0, 68
	split 1, 77
	split 2, 83
	split 3, 88

keysplit qz_songs_cello, 41
	split 0, 58
	split 1, 68
	split 2, 90

keysplit qz_songs_synth_brass_2, 38
	split 0, 49
	split 1, 62

keysplit qz_songs_baritone_sax, 35
	split 0, 56
	split 1, 65
	split 2, 76

keysplit qz_songs_square_lead, 32
	split 0, 42
	split 1, 51
	split 2, 60
	split 3, 68
	split 4, 77
	split 5, 85
	split 6, 102

@ end of Johto music
