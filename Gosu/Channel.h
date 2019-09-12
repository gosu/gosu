#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Gosu_Channel Gosu_Channel;

Gosu_Channel* Gosu_Channel_create(Gosu_Channel* channel);
void Gosu_Channel_destroy(Gosu_Channel* channel);

void Gosu_Channel_play(Gosu_Channel *channel);
bool Gosu_Channel_playing(Gosu_Channel *channel);
void Gosu_Channel_pause(Gosu_Channel *channel);
bool Gosu_Channel_paused(Gosu_Channel *channel);
void Gosu_Channel_resume(Gosu_Channel *channel);
void Gosu_Channel_stop(Gosu_Channel *channel);

void Gosu_Channel_set_volume(Gosu_Channel *channel, double volume);
void Gosu_Channel_set_speed(Gosu_Channel *channel, double speed);
void Gosu_Channel_set_pan(Gosu_Channel *channel, double pan);

#ifdef __cplusplus
}
#endif