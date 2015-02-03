#ifndef SOUND_LIB
#define SOUND_LIB

#define DEFAULT_PLAY_TIMES 1
#define play_wave_default(filename) play_wave(filename, DEFAULT_PLAY_TIMES)

extern int play_wave(const char *filename, int refresh_times);


#endif
