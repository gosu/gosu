#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_Song Gosu_Song;

Gosu_Song* Gosu_Song_create(const char* filename);
void Gosu_Song_destroy(Gosu_Song* song);

void Gosu_Song_pause(Gosu_Song* song);
bool Gosu_Song_paused(Gosu_Song* song);
void Gosu_Song_play(Gosu_Song* song, bool looping);
bool Gosu_Song_playing(Gosu_Song* song);
void Gosu_Song_stop(Gosu_Song* song);
double Gosu_Song_volume(Gosu_Song* song);
void Gosu_Song_set_volume(Gosu_Song* song, double volume);

Gosu_Song* Gosu_Song_current_song();

#ifdef __cplusplus
}
#endif