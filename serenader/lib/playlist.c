#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "logging.h"
#include "playlist.h"

#undef PLAYLIST_SIZE
#define PLAYLIST_SIZE (6)

static char const *IM_ON_FIRE[] = {
    "[Now playing... I'm on Fire, by Bruce Springsteen]",
    "",
    "Hey, little girl, is your daddy home?",
    "Did he go away and leave you all alone?",
    "I got a bad desire",
    "Oh, oh, oh",
    "I'm on fire",
    "Tell me now, baby, is he good to you?",
    "And can he do to you the things that I do?",
    "Oh no, I can take you higher",
    "Oh, oh, oh",
    "I'm on fire",
    "Sometimes it's like someone took a knife, baby, edgy and dull",
    "And cut a six-inch valley through the middle of my skull",
    "At night, I wake up with the sheets soakin' wet",
    "And a freight train runnin' through the middle of my head",
    "Only you can cool my desire",
    "",
    NULL};

static char const *BOOTS_OF_SPANISH_LEATHER[] = {
    "[Now playing... Boots of Spanish Leather, by Bob Dylan]",
    "",
    "Oh, I’m sailin' away my own true love",
    "I’m a-sailin' away in the morning",
    "Is there somethin' I can send you from across the sea",
    "From the place that I’ll be landing?",
    "No, there’s nothin' you can send me, my own true love",
    "There’s nothin' I'm a-wishing to be ownin'",
    "Just a-carry yourself back to me unspoiled",
    "From across that lonesome ocean",
    "Ah, but I just thought you might want somethin' fine",
    "Made of silver or of golden",
    "Either from the mountains of Madrid",
    "Or from the coast of Barcelona",
    "Well, if I had the stars from the darkest night",
    "And the diamonds from the deepest ocean",
    "I’d forsake them all for your sweet kiss",
    "For that’s all I’m a-wishin' to be ownin'",
    "Well I might be gone a long old time",
    "And it’s only that I’m asking",
    "Is there somethin' I can send you to remember me by",
    "To make your time more easy passing",
    "Oh, how can, how can you ask me again",
    "It only brings me sorrow",
    "The same thing I would want today",
    "I want again tomorrow",
    "I got a letter on a lonesome day",
    "It was from her ship a-sailin'",
    "Sayin' I don’t know when I’ll be comin' back again",
    "It depends on how I’m a-feelin'",
    "If a-you, my love, must think that-a-way",
    "I’m sure your mind is a-roamin'",
    "I’m sure your thoughts are not with me",
    "But with the country to where you’re goin'",
    "So take heed, take heed of the western winds",
    "Take heed of the stormy weather",
    "And yes, there’s somethin' you can send back to me",
    "Spanish boots of Spanish leather",
    "",
    NULL};
static char const *BROWN_EYED_WOMEN[] = {
    "[Now playing... Brown Eyed Women, by the Grateful Dead (and without Bob "
    "Dylan)]",
    "",
    "Gone are the days when the ox fall down",
    "Take up the yoke and plow the fields around",
    "Gone are the days when the ladies said \"Please",
    "Gentle Jack Jones, won't you come to me\"",
    "Brown-eyed women and red grenadine",
    "The bottle was dusty, but the liquor was clean",
    "Sound of the thunder with the rain pourin' down",
    "And it looks like the old man's gettin' on",
    "1920 when he stepped to the bar",
    "Drank to the dregs of the whiskey jar",
    "1930 when the wall caved in",
    "He made his way selling red-eyed gin",
    "Brown-eyed women and red grenadine",
    "The bottle was dusty, but the liquor was clean",
    "Sound of the thunder with the rain pourin' down",
    "And it looks like the old man's gettin' on",
    "Delilah Jones was the mother of twins",
    "Two times over, and the rest were sins",
    "Raised eight boys, only I turned bad",
    "Didn't get the lickin's that the other ones had",
    "Brown-eyed women and red grenadine",
    "The bottle was dusty, but the liquor was clean",
    "Sound of the thunder with the rain pourin' down",
    "And it looks like the old man's gettin' on",
    "Tumble down shack on Big Foot county",
    "Snowed so hard that the roof caved in",
    "Delilah Jones went to meet her God",
    "And the old man never was the same again",
    "Daddy made whiskey and he made it well",
    "Cost two dollars and it burned like hell",
    "I cut hick'ry just to fire the still",
    "Drink down a bottle and be ready to kill",
    "Brown-eyed women and red grenadine",
    "The bottle was dusty, but the liquor was clean",
    "Sound of the thunder with the rain pourin' down",
    "And it looks like the old man's gettin' on",
    "Gone are the days when the ox fall down",
    "Take up the yoke and plow the fields around",
    "Gone are the days when the ladies said \"Please",
    "Gentle Jack Jones, won't you come to me?\"",
    "Brown-eyed women and red grenadine",
    "The bottle was dusty, but the liquor was clean",
    "Sound of the thunder with the rain pourin' down",
    "And it looks like the old man's gettin' on",
    "",
    NULL};
static char const *MAMMA_YOUVE_BEEN_ON_MY_MIND[] = {
    "[Now playing... Mamma You Been on My Mind, by Jeff Buckley]",
    "",
    "Perhaps it is the color of the sun cut flat",
    "And covering the crossroads I'm standing at",
    "Or maybe it's the weather or something like that",
    "But mama, you've been on my mind",
    "I mean no trouble, please don't put me down, don't get upset",
    "I am not pleading, or saying I can't forget you",
    "I do not pace the floor, bowed down and bent, but yet",
    "Well, mama you've been on my mind",
    "Even though my eyes are hazy and my thoughts they might be narrow",
    "Where you've been don't bother me, or bring me down in sorrow",
    "I don't even mind who you'll be waking with tomorrow",
    "But mama, you're just on my mind",
    "I am not askin' you to say words like yes or no",
    "Please understand me, I have no place, I'm calling you to go",
    "I'm just whispering to myself so I can pretend that I don't know",
    "Mama, you're just on my mind",
    "Well, mama you're just on my mind",
    "When you wake up in the morning, baby look inside your mirror",
    "You know I won't be next to, you know I won't be near",
    "I'd just be curious to know if you can see yourself as clear",
    "As someone who has had you on his mind",
    "",
    NULL};
static char const *LOVE_OF_MY_LIFE[] = {
    "[Now playing... Love of my Life, by Queen]",
    "",
    "Love of my life, you've hurt me",
    "You've broken my heart",
    "And now you leave me",
    "Love of my life, can't you see?",
    "Bring it back, bring it back",
    "Don't take it away from me",
    "Because you don't know",
    "What it means to me",
    "Love of my life, don't leave me",
    "You've taken my love (all my love)",
    "And now desert me",
    "Love of my life, can't you see?",
    "(Please bring it back, back) Bring it back, bring it back",
    "Don't take it away from me",
    "Because you don't know",
    "What it means to me (means to me)",
    "You will remember when this is blown over",
    "And everything's all by the way",
    "When I grow older",
    "I will be there at your side",
    "To remind you",
    "How I still love you (I still love you)",
    "Back, hurry back",
    "Please bring it back home to me",
    "Because you don't know",
    "What it means to me (means to me)",
    "Love of my life",
    "Love of my life",
    "Ooh, ooh",
    "",
    NULL};
static char const *ESCAPE[] = {
    "[Now playing... Escape (the Piña Colada Song), by Rupert Holmes]",
    "",
    "I was tired of my lady",
    "We'd been together too long",
    "Like a worn out recording",
    "Of a favorite song",
    "So while she lay there sleeping",
    "I read the paper in bed",
    "And in the personal columns",
    "There was this letter I read:",
    "\"If you like piña coladas",
    "And getting caught in the rain",
    "If you're not into yoga",
    "If you have half a brain",
    "If you like making love at midnight",
    "In the dunes on the cape",
    "Then I'm the love that you've looked for",
    "Write to me and escape\"",
    "I didn't think about my lady",
    "I know that sounds kind of mean",
    "But me and my old lady",
    "Had fallen into the same old dull routine",
    "So I wrote to the paper",
    "Took out a personal ad",
    "And though I'm nobody's poet",
    "I thought it wasn't half bad:",
    "\"Yes, I like piña coladas",
    "And getting caught in the rain",
    "I'm not much into health food",
    "I am into champagne",
    "I've got to meet you by tomorrow noon",
    "And cut through all this red tape",
    "At a bar called O'Malley's",
    "Where we'll plan our escape\"",
    "So I waited with high hopes",
    "And she walked in the place",
    "I knew her smile in an instant",
    "I knew the curve of her face",
    "It was my own lovely lady",
    "And she said, \"Aw, it's you\"",
    "Then we laughed for a moment",
    "And I said, \"I never knew...\"",
    "\"...That you like piña coladas",
    "And gettin' caught in the rain",
    "And the feel of the ocean",
    "And the taste of champagne",
    "If you like making love at midnight",
    "In the dunes on the cape",
    "You're the lady I've looked for",
    "Come with me and escape\"",
    "If you like piña coladas",
    "And getting caught in the rain",
    "If you're not into yoga",
    "If you have half a brain",
    "If you like making love at midnight",
    "In the dunes on the cape",
    "Then I'm the love that you've looked for",
    "Write to me and escape",
    "Yes, I like piña coladas",
    "And getting caught in the rain",
    "I'm not much into health food",
    "I am into champagne",
    "I've got to meet you by tomorrow noon",
    "And cut through all this red tape",
    "At a bar called O'Malley's",
    "Where we'll plan our escape",
    "",
    NULL};

static char const **PLAYLIST[] = {MAMMA_YOUVE_BEEN_ON_MY_MIND,
                                  IM_ON_FIRE,
                                  BOOTS_OF_SPANISH_LEATHER,
                                  BROWN_EYED_WOMEN,
                                  LOVE_OF_MY_LIFE,
                                  ESCAPE};

// ========================
//  global state
// ========================

char *g_nickname = NULL;
size_t g_song_idx = 0;
size_t g_verse_idx = 0;
size_t g_played_songs = 0;
bool g_paused = true;
bool g_repeat = false;
bool g_shuffle = false;
bool g_finished = true;
bool g_new_song = false;

struct timespec g_last_verse_time = {.tv_sec = 0, .tv_nsec = 0};

// ========================
//  helpers
// ========================

static size_t time_elapsed_ms(struct timespec const *since) {
    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
        WARN("failed to get current time: %s", strerror(errno));
        return 0;
    }

    if (since->tv_sec > now.tv_sec) {
        // clock went backwards
        return 0;
    }

    ssize_t sec_diff = (ssize_t)now.tv_sec - (ssize_t)since->tv_sec;
    ssize_t nsec_diff = (ssize_t)now.tv_nsec - (ssize_t)since->tv_nsec;

    return (size_t)((sec_diff * 1000) + (nsec_diff / 1000000));
}

static size_t next_song(size_t current_song) {
    if (PLAYLIST_SIZE < 2) {
        return current_song;
    }

    int inc = 1;
    if (g_shuffle) {
        inc = rand();
        if (inc % PLAYLIST_SIZE == 0) {
            inc = 2;
        }
    }

    return (current_song + (size_t)inc) % PLAYLIST_SIZE;
}

// ========================
//  library implementation
// ========================

bool playlist_is_finished() { return g_finished; }

bool playlist_is_playing() {
    return g_nickname != NULL && !g_paused && !playlist_is_finished();
}

char const *playlist_current_nickname() { return g_nickname; }

// try to start the playlist; -1 if playlist is running
int playlist_play(char const *nickname, size_t nick_len) {
    if (g_nickname != NULL) {
        WARN("please apologize to %.*s, already serenading on %s\n",
             (int)nick_len, nickname, g_nickname);
        return -1;
    }

    g_nickname = calloc(nick_len + 1, sizeof(char));
    memcpy(g_nickname, nickname, nick_len);

    g_finished = false;
    g_paused = false;
    g_new_song = true;
    srand((unsigned int)time(NULL));
    return 0;
}

// stop playing
void playlist_stop() {
    if (g_nickname != NULL) {
        free(g_nickname);
        g_nickname = NULL;

        g_finished = true;
        g_paused = true;
        g_played_songs = 0;
        g_repeat = false;
        g_shuffle = false;
        g_song_idx = 0;
        g_verse_idx = 0;
        g_new_song = false;
    }
}

// pause/resume the playlist execution
void playlist_pause() {
    if (g_nickname != NULL) {
        g_paused = !g_paused;
        if (!g_repeat) {
            g_finished = false;
        }
    }
}

// toggle the repeat
void playlist_toggle_repeat() {
    if (g_nickname != NULL) {
        g_repeat = !g_repeat;
        if (g_repeat) {
            g_finished = false;
        }
    }
}

// toggle the shuffle
void playlist_toggle_shuffle() {
    if (g_nickname != NULL) {
        g_shuffle = !g_shuffle;
    }
}

// get next verse (if any)
// may return null
char const *playlist_next_verse() {
    // not playing
    if (!playlist_is_playing()) {
        return NULL;
    }

    // go to next song
    if (PLAYLIST[g_song_idx][g_verse_idx] == NULL) {
        g_verse_idx = 0;
        g_played_songs += 1;
        g_finished = !g_repeat && g_played_songs > 0 &&
                     g_played_songs % PLAYLIST_SIZE == 0;
        g_song_idx = next_song(g_song_idx);
        g_new_song = true;
    }

    if (playlist_is_finished()) {
        return NULL;
    }

    // have not finished the delay
    if ((g_new_song && time_elapsed_ms(&g_last_verse_time) < SONG_DELAY_MS) ||
        (!g_new_song && time_elapsed_ms(&g_last_verse_time) < VERSE_DELAY_MS)) {
        return NULL;
    }

    // finished the delay, reset
    clock_gettime(CLOCK_REALTIME, &g_last_verse_time);

    char const *verse = PLAYLIST[g_song_idx][g_verse_idx];
    g_verse_idx += 1;
    g_new_song = false;
    return verse;
}
