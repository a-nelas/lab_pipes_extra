#ifndef __SERENADER_PLAYLIST_H__
#define __SERENADER_PLAYLIST_H__

#include <stdbool.h>

#define SONG_DELAY_MS (1000)
#define VERSE_DELAY_MS (500)

// whether the playlist finished playing
bool playlist_is_finished();

// whether a song is playing currently
bool playlist_is_playing();

char const *playlist_current_nickname();

// try to start the playlist; -1 if playlist is running
int playlist_play(char const *nickname, size_t nick_len);

// stop playing
void playlist_stop();

// pause/resume the playlist execution
void playlist_pause();

// toggle the repeat
void playlist_toggle_repeat();

// toggle the shuffle
void playlist_toggle_shuffle();

// get next verse (if any)
// may return null
char const *playlist_next_verse();

#endif // __SERENADER_PLAYLIST_H__
