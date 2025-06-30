#include <lauxlib.h>
#include <lua.h>
#include <lua.hpp>
#include <raylib.h>
#include "../../../libs/lua_ffi.hpp" // for pushPtr, getPtr
#include <algorithm>     // std::remove
#include <vector>

struct SoundWraper{
	Sound sound;
	float volume = 1.0f;
};

struct MusicWraper {
	Music music;
	float volume = 1.0f;
};

static std::vector<SoundWraper*> soundPool;
static std::vector<MusicWraper*> musicPool;

static int l_LoadSound(lua_State *L) {
	const char *filename = luaL_checkstring(L, 1);
	Sound sound = LoadSound(filename);
	if(!IsSoundValid(sound))
	{
		lua_pushnil(L);
		lua_pushstring(L, "Failed to load sound");
		return 2;
	}

	SoundWraper* sw = new SoundWraper{sound};
	soundPool.push_back(sw);

	pushPtr<SoundWraper>(L, sw);
	return 1;
}

static int l_UnloadSound(lua_State *L) {
	SoundWraper* sw = getPtr<SoundWraper>(L, 1);
	if(!sw)
		return 0;

	if(IsSoundPlaying(sw->sound))
		StopSound(sw->sound);

	if(sw->sound.stream.buffer != NULL)
		UnloadSound(sw->sound);

	soundPool.erase(std::remove(soundPool.begin(), soundPool.end(), sw), soundPool.end());

	delete sw;
	return 0;
}

static int l_PlaySound(lua_State *L) {
	SoundWraper* sw = getPtr<SoundWraper>(L, 1);
	if(!sw)
		return 0;

	PlaySound(sw->sound);
	return 0;
}

static int l_StopSound(lua_State *L) {
	SoundWraper* sw = getPtr<SoundWraper>(L, 1);
	if(!sw)
		return 0;

	StopSound(sw->sound);
	return 0;
}

static int l_AudioDeviceInit(lua_State *L) {
	InitAudioDevice();
	return 0;
}

static int l_AudioDeviceClose(lua_State *L) {
	for(SoundWraper* sw : soundPool){
		UnloadSound(sw->sound);
		delete sw;
	}

	for(MusicWraper* mw : musicPool){
		UnloadMusicStream(mw->music);
		delete mw;
	}

	soundPool.clear();
	musicPool.clear();
	CloseAudioDevice();
	return 0;
}
static int l_IsSoundReady(lua_State *L) {
	SoundWraper* sw = getPtr<SoundWraper>(L, 1);
	lua_pushboolean(L, sw && sw->sound.stream.buffer != NULL);
	return 1;
}

static int l_AudioVolume(lua_State *L) {
	// if there's only the sound, return the volume
	if(lua_gettop(L) == 1){
		SoundWraper* sw = getPtr<SoundWraper>(L, 1);
		lua_pushnumber(L, sw->volume);
		return 1;
	}

	SoundWraper* sw = getPtr<SoundWraper>(L, 1);
	float volume = luaL_checknumber(L, 2);
	SetSoundVolume(sw->sound, volume);
	sw->volume = volume;
	return 0;
}

void registerSoundClass(lua_State *L) {
	const char* type = typeid(SoundWraper).name();
	if(luaL_newmetatable(L, type)){
		lua_pushstring(L, "__index");
		lua_newtable(L);
		
		lua_pushcfunction(L, l_PlaySound);
		lua_setfield(L, -2, "Play");
		
		lua_pushcfunction(L, l_StopSound);
		lua_setfield(L, -2, "Stop");

		lua_pushcfunction(L, l_UnloadSound);
		lua_setfield(L, -2, "Unload");

		lua_pushcfunction(L, l_IsSoundReady);
		lua_setfield(L, -2, "IsReady");

		lua_pushcfunction(L, l_AudioVolume);
		lua_setfield(L, -2, "Volume");

		lua_settable(L, -3);
	}

	lua_pop(L, 1);
}
static int l_PlayMusic(lua_State *L) {
	MusicWraper* mw = getPtr<MusicWraper>(L, 1);
	if(!mw) return 0;
	PlayMusicStream(mw->music);
	return 0;
}

static int l_StopMusic(lua_State *L) {
	MusicWraper* mw = getPtr<MusicWraper>(L, 1);
	if(!mw) return 0;
	StopMusicStream(mw->music);
	return 0;
}

static int l_PauseMusic(lua_State *L) {
	MusicWraper* mw = getPtr<MusicWraper>(L, 1);
	if(!mw) return 0;
	PauseMusicStream(mw->music);
	return 0;
}

static int l_ResumeMusic(lua_State *L) {
	MusicWraper* mw = getPtr<MusicWraper>(L, 1);
	if(!mw) return 0;
	ResumeMusicStream(mw->music);
	return 0;
}

static int l_UpdateMusic(lua_State *L) {
	MusicWraper* mw = getPtr<MusicWraper>(L, 1);
	if(!mw) return 0;
	UpdateMusicStream(mw->music);
	return 0;
}

static int l_IsMusicReady(lua_State *L) {
	MusicWraper* mw = getPtr<MusicWraper>(L, 1);
	lua_pushboolean(L, mw && mw->music.ctxData != NULL);
	return 1;
}

static int l_MusicVolume(lua_State *L) {
	MusicWraper* mw = getPtr<MusicWraper>(L, 1);
	if(!mw) return 0;

	if(lua_gettop(L) == 1) {
		lua_pushnumber(L, mw->volume);
		return 1;
	}

	float volume = luaL_checknumber(L, 2);
	SetMusicVolume(mw->music, volume);
	mw->volume = volume;
	return 0;
}

static int l_UnloadMusic(lua_State *L) {
	MusicWraper* mw = getPtr<MusicWraper>(L, 1);
	if(!mw) return 0;

	UnloadMusicStream(mw->music);
	musicPool.erase(std::remove(musicPool.begin(), musicPool.end(), mw), musicPool.end());
	delete mw;
	return 0;
}
void registerMusicClass(lua_State *L) {
	const char* type = typeid(MusicWraper).name();
	if(luaL_newmetatable(L, type)) {
		lua_pushstring(L, "__index");
		lua_newtable(L);

		lua_pushcfunction(L, l_PlayMusic);
		lua_setfield(L, -2, "Play");

		lua_pushcfunction(L, l_StopMusic);
		lua_setfield(L, -2, "Stop");

		lua_pushcfunction(L, l_PauseMusic);
		lua_setfield(L, -2, "Pause");

		lua_pushcfunction(L, l_ResumeMusic);
		lua_setfield(L, -2, "Resume");

		lua_pushcfunction(L, l_UpdateMusic);
		lua_setfield(L, -2, "Update");

		lua_pushcfunction(L, l_IsMusicReady);
		lua_setfield(L, -2, "IsReady");

		lua_pushcfunction(L, l_MusicVolume);
		lua_setfield(L, -2, "Volume");

		lua_pushcfunction(L, l_UnloadMusic);
		lua_setfield(L, -2, "Unload");

		lua_settable(L, -3);
	}

	lua_pop(L, 1);
}

void init_raylib_sound(lua_State *L) {
	registerSoundClass(L);
	registerMusicClass(L);
	static luaL_Reg funcs[] = {
		{"init", l_AudioDeviceInit},
		{"close", l_AudioDeviceClose},
		{"loadSound", l_LoadSound},
		{"loadMusic", l_LoadSound},

		{NULL, NULL}
	};
	newModule("Sound", funcs, L);

}