//
// Created by Gegel85 on 31/10/2020
//

#define _USE_MATH_DEFINES

#include <fstream>
#include <sstream>
#include <optional>
#include <dinput.h>
#include <nlohmann/json.hpp>
#include <SokuLib.hpp>
#include <shlwapi.h>
#include <thread>
#include <deque>
#include "CustomPackets.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif


#define Dequeue Deque
#define FONT_HEIGHT 16
#define TEXTURE_SIZE 0x200
#define BOXES_ALPHA 0.25

#define HEALING_INVUL 90
#define HEALING_TIME 180
#define HEALTH_PERCENT (15.f / 100.f)
#define HEAL_RADIUS (100)

#define sendPacket(p) do { \
	auto &__netobj = SokuLib::getNetObject(); \
	SokuLib::DLL::ws2_32.sendto(__netobj.netUdp.socket, (char *)&(p), sizeof(p), 0, (sockaddr *)&__netobj.netUdp.opponent, sizeof(__netobj.netUdp.opponent));                     \
} while (false)


const char emptyMagicString[32] = "\0\x1F\x54\xF2\xA2\x67\x90\x78\xC2";
const char *emptyName = "<color FF8000>[Empty Slot]</color>";

// Contructor:
// void FUN_0047f070(HudPlayerState *param_1, char param_1_00, CDesignBase *param_3, CDesignBase *param_4, int param_5)
// FUN_0047f070(&this->p1state, 0,(CDesignBase *)&this->field_0x98,(CDesignBase *)&this->field_0xcc,&this->field_0x4);
// Init:
// void FUN_0047ede0(HudPlayerState *param_1, Player *param_1_00)
// FUN_0047ede0(this, player);
struct HudPlayerState {
	char offset_0x00[0xC];
	unsigned hp;
	unsigned lastHp;
	char offset_0x14[0x4];
	SokuLib::CharacterManager *player;
	char offset_0x1C[0xDC];
};

// Contructor: 
// Init:
// void FUN_00435f10(HudPlayerStateUnknown2 *this, DeckInfo *deckInfo, int index)
// FUN_00435f10(this, &player->deckInfo, playerIndex);
struct HudPlayerState2 {
	SokuLib::DeckInfo *deck;
	char offset_0x000[0x150];
};

// Contructor:
// Init:
// void FUN_00478c50 (HudPlayerState3 *this, CDesignBase *obj, Player *player)
// FUN_00478c50(this_00, (CDesignBase *)&this->field_0x100, player);
struct HudPlayerState3 {
	void *designObject;
	SokuLib::CharacterManager *player;
	char offset_0x000[0x2C];
};

struct CInfoManager {
	void **vtable;
	char offset_0x004[0x168];
	HudPlayerState3 p1State3;
	HudPlayerState3 p2State3;
	HudPlayerState2 p1State2;
	HudPlayerState2 p2State2;
	SokuLib::Sprite *p1Portrait;
	SokuLib::Sprite *p2Portrait;
	char offset_0x484[0x14];
	HudPlayerState p1State;
	HudPlayerState p2State;
};

struct Deck {
	std::string name;
	std::array<unsigned short, 20> cards;
};

struct Guide {
	bool active = false;
	SokuLib::DrawUtils::Sprite sprite;
	unsigned char alpha = 0;
};

static_assert(offsetof(CInfoManager, p1State3) == 0x16C);
static_assert(offsetof(CInfoManager, p2State3) == 0x1A0);
static_assert(offsetof(CInfoManager, p1Portrait) == 0x47C);
static_assert(offsetof(CInfoManager, p2Portrait) == 0x480);
static_assert(offsetof(CInfoManager, p1State) == 0x498);
static_assert(offsetof(CInfoManager, p2State) == 0x590);

static_assert(offsetof(HudPlayerState, hp) == 0xC);
static_assert(offsetof(HudPlayerState, lastHp) == 0x10);
static_assert(offsetof(HudPlayerState, player) == 0x18);


struct GameDataManager {
	// 0x00
	SokuLib::List<std::pair<int, SokuLib::PlayerInfo*>> createQueue;
	// 0x0C
	bool isThreadsRunning; // align 3
	// 0x10
	uint32_t threadHandleA; // 0x46e6d0 
	uint32_t threadIdA;
	uint32_t threadHandleB; // 0x46e6f0 THREAD_PRIORITY_IDLE
	uint32_t threadIdB;
	uint32_t eventHandleA;
	uint32_t eventHandleB;
	// 0x28
	SokuLib::CharacterManager* players[4];
	// 0x38
	bool enabledPlayers[4]; // 01 01 00 00
	// 0x3C
	SokuLib::Vector<SokuLib::CharacterManager*> activePlayers;
	// 0x4C
	SokuLib::List<SokuLib::CharacterManager*> destroyQueue;
}; // 0x58

struct Coord { float x, y, z; };
struct SpriteEx {
	void *vtable;
	int dxHandle = 0;
	SokuLib::DxVertex vertices[4];
	SokuLib::Vector2f size;
	Coord baseCoords[4];
	Coord transfCoords[4];
	SokuLib::Vector2f size2;

	void setTexture(int texture, int texOffsetX, int texOffsetY, int width, int height, int anchorX, int anchorY);
	void setTexture(int texture, int texOffsetX, int texOffsetY, int width, int height);
	void clearTransform();
	void render();
	void render(float r, float g, float b);
	void render(float a, float r, float g, float b);
};
void SpriteEx::setTexture(int texture, int texOffsetX, int texOffsetY, int width, int height, int anchorX, int anchorY) {
	(this->*SokuLib::union_cast<void(SpriteEx::*)(int, int, int, int, int, int, int)>(0x406c60))(texture, texOffsetX, texOffsetY, width, height, anchorX, anchorY);
}
void SpriteEx::setTexture(int texture, int texOffsetX, int texOffsetY, int width, int height) {
	(this->*SokuLib::union_cast<void(SpriteEx::*)(int, int, int, int, int)>(0x41f7f0))(texture, texOffsetX, texOffsetY, width, height);
}
void SpriteEx::clearTransform() { (this->*SokuLib::union_cast<void(SpriteEx::*)()>(0x406ea0))(); }
void SpriteEx::render() { (this->*SokuLib::union_cast<void(SpriteEx::*)()>(0x4075d0))(); }
void SpriteEx::render(float r, float g, float b) { (this->*SokuLib::union_cast<void(SpriteEx::*)(float, float, float)>(0x7fb080))(r, g, b); }
void SpriteEx::render(float a, float r, float g, float b) { (this->*SokuLib::union_cast<void(SpriteEx::*)(float, float, float, float)>(0x7fb150))(a, r, g, b); }

struct SWRCMDINFO {
	bool enabled;
	int prev; // number representing the previously pressed buttons (masks are applied)
	int now; // number representing the current pressed buttons (masks are applied)

	struct {
		bool enabled;
		int id[10];
		int base; // once len reaches 10 (first cycle), is incremented modulo 10
		int len; // starts at 0, caps at 10
	} record;
};

struct RivControl {
	bool enabled;
	int texID;
	int forwardCount;
	int forwardStep;
	int forwardIndex;
	SWRCMDINFO cmdp1;
	SWRCMDINFO cmdp2;
	bool hitboxes;
	bool untech;
	bool show_debug;
	bool paused;
};

static void (*og_loadDat)(const char *path);
static unsigned char (__fastcall *og_advanceFrame)(SokuLib::CharacterManager *);
static void (*s_originalDrawGradiantBar)(float param1, float param2, float param3);
static int (SokuLib::SelectClient::*s_originalSelectClientOnProcess)();
static int (SokuLib::SelectClient::*s_originalSelectClientOnRender)();
static int (SokuLib::Select::*s_originalSelectOnProcess)();
static int (SokuLib::Select::*s_originalSelectOnRender)();
static int (SokuLib::Title::*s_originalTitleOnProcess)();
static int (SokuLib::BattleManager::*s_originalBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*s_originalBattleMgrOnRender)();
static int (SokuLib::ProfileDeckEdit::*s_originalCProfileDeckEdit_OnProcess)();
static int (SokuLib::ProfileDeckEdit::*s_originalCProfileDeckEdit_OnRender)();
static SokuLib::ProfileDeckEdit *(SokuLib::ProfileDeckEdit::*s_originalCProfileDeckEdit_Destructor)(unsigned char param);
static SokuLib::ProfileDeckEdit *(SokuLib::ProfileDeckEdit::*og_CProfileDeckEdit_Init)(int param_2, int param_3, SokuLib::Sprite *param_4);
static void (__fastcall *og_handleInputs)(SokuLib::CharacterManager *);
static void (__fastcall *ogBattleMgrHandleCollision)(SokuLib::BattleManager*, int, void*, SokuLib::CharacterManager*);
static void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::DeckInfo &, int, SokuLib::Dequeue<short> &);
static int (*og_FUN4098D6)();
static void (__fastcall *og_FUNCALL43F996)(void *);

static GameDataManager*& dataMgr = *(GameDataManager**)SokuLib::ADDR_GAME_DATA_MANAGER;

struct CEffectManager {
	void **vtable;
	void **vtable2;
	char offset_0x8[0x90];
};

struct ExtraChrSelectData {
	SokuLib::CDesign::Object *name;
	SokuLib::CDesign::Object *portrait;
	SokuLib::CDesign::Object *spellCircle;
	SokuLib::CDesign::Object *charObject;
	SokuLib::CDesign::Object *deckObject;
	SokuLib::CDesign::Sprite *profileBack;
	SokuLib::CDesign::Object *gear;
	SokuLib::CDesign::Sprite *cursor;
	SokuLib::CDesign::Sprite *deckSelect;
	SokuLib::CDesign::Sprite *colorSelect;
	SokuLib::v2::AnimationObject *object;
	SokuLib::KeymapManager *input;
	SokuLib::InputHandler chrHandler;
	SokuLib::InputHandler palHandler;
	SokuLib::InputHandler deckHandler;
	SokuLib::Sprite portraitSprite;
	SokuLib::Sprite circleSprite;
	SokuLib::Sprite gearSprite;
	CEffectManager effectMgr;
	int portraitTexture = 0;
	int circleTexture = 0;
	int selectState;
	int charNameCounter;
	int portraitCounter;
	int cursorCounter;
	int deckIndCounter;
	int chrCounter;
	bool needInit;
	bool isInit = false;
};

//static SokuLib::CharacterManager *obj[0xC] = {nullptr};
static char extraProfiles[0x7EC + 0x7EC];
static SokuLib::Profile *profiles[4] = {
	&SokuLib::profile1,
	&SokuLib::profile2,
	(SokuLib::Profile *)&extraProfiles[0],
	(SokuLib::Profile *)&extraProfiles[0x7EC],
};
static SokuLib::CDesign::Sprite *profileHandlers[2];
static char modFolder[1024];
static char nameBuffer[64];
static char soku2Dir[MAX_PATH];
static SokuLib::DrawUtils::RectangleShape rectangle;
static bool spawned = false;
static bool init = false;
static bool disp = false;
static bool anim = false;
static HMODULE myModule;
static SokuLib::DrawUtils::Sprite healingCircle[2];
static SokuLib::DrawUtils::Sprite cursors[4];
static SokuLib::DrawUtils::Sprite cursors2[4];
static bool assetsLoaded = false;
static CInfoManager hud2;
static bool hudInit = false;
static ExtraChrSelectData chrSelectExtra[2];
static SokuLib::KeymapManager *chrSelectExtraInputs[2] = {nullptr, nullptr};
static SokuLib::Character lastChrs[4];
static bool generated = false;
static int selectedDecks[2] = {0, 0};
static int editSelectedProfile = 0;
static bool displayCards = true;
static bool profileSelectReady = false;
static bool hasSoku2 = false;
static int counter = 0;
static SokuLib::SWRFont defaultFont;
static SokuLib::SWRFont font;
static std::unique_ptr<std::array<unsigned short, 20>> fakeDeck[4];
static unsigned char errorCounter = 0;
static SokuLib::DrawUtils::Sprite arrowSprite;
static SokuLib::DrawUtils::Sprite baseSprite;
static SokuLib::DrawUtils::Sprite nameSprite;
static SokuLib::DrawUtils::Sprite noSprite;
static SokuLib::DrawUtils::Sprite noSelectedSprite;
static SokuLib::DrawUtils::Sprite yesSprite;
static SokuLib::DrawUtils::Sprite yesSelectedSprite;
static Guide createDeckGuide;
static Guide selectDeckGuide;
static Guide editBoxGuide;
static bool copyBoxDisplayed = false;
static bool renameBoxDisplayed = false;
static bool deleteBoxDisplayed = false;
static unsigned editSelectedDeck = 0;
static std::vector<Deck> editedDecks;
static char editingBoxObject[0x164];
static unsigned char copyBoxSelectedItem = 0;
static unsigned char deleteBoxSelectedItem = 0;
static bool saveError = false;
static bool escPressed = false;
static bool revived = false;
static std::string lastLoadedProfile;
static std::string loadedProfiles[4];
static SokuLib::Sprite extraNetProfileNames[2];
static int extraNetProfileNamesTextures[2];
static char extraOnlineNames[2][32];
static bool characterSelectLocked = false;
static char mySlot = -1;

// Which of the four seats this client occupies, for other mods -- giuroll needs
// it to route each peer's inputs to the right player.
//
// The server assigns it (PacketPlayerJoinAck) and nothing else can work it out:
// a rollback netcode that guesses which slot it is puts every remote player in
// control of the wrong character. Exported rather than published at a fixed
// address because this DLL's base moves between launches.
//
// Returns -1 when no slot is held: not in a 4P session, or not yet joined.
extern "C" __declspec(dllexport) int FourPSokuLocalSlot()
{
	return mySlot;
}
static char remoteMySlot = -1;
static char selectedSlot = -1;
static bool slotsTaken[4] = {false, false, false, false};
static SokuLib::DrawUtils::Sprite chooseBoxBack;
static SokuLib::DrawUtils::Sprite chooseBoxText;
static SokuLib::DrawUtils::Sprite chooseBoxWaitText;
static SokuLib::KeymapManager extraOnlineInputs[2] = {{(void *)0x85844C, {}, {}, {}, {}, true}, {(void *)0x85844C, {}, {}, {}, {}, true}};
static std::deque<unsigned short> extraOnlineInputsBuffer[2];
static std::vector<std::array<SokuLib::Inputs, 4>> replayInputs;

// --- 4P replay sidecar -----------------------------------------------------
//
// Soku's own .rep holds two inputs per frame and nothing here changes that: a
// 4P replay stays a VALID 2P replay that plays P1/P2 correctly in every
// existing tool. P3/P4 ride alongside in a sidecar file.
//
// WHY A SIDECAR RATHER THAN A WIDER .rep. Extending the real format means
// reverse-engineering the writer (SokuLib does not wrap it -- only
// readReplay is exposed) and then breaking every replay tool in the ecosystem.
// The sidecar costs one file and keeps both properties.
//
// WHY THIS EXISTS AT ALL, given replays were deferred: giuroll's rollback
// verifier (check mode / PERFORMANCE_TEST) only runs on replays -- in local
// versus giuroll's per-frame dispatch falls through to a no-op and no
// savestate machinery runs at all. Replays are the only existing driver for
// forcing a rollback, so proving the savestate covers P3/P4 needs them.
static SokuLib::KeymapManager replayExtraInputs[2] = {{(void *)0x85844C, {}, {}, {}, {}, true}, {(void *)0x85844C, {}, {}, {}, {}, true}};
static std::vector<std::array<unsigned short, 2>> sidecarInputs;
static size_t sidecarCursor = 0;
static bool sidecarLoaded = false;

// KeyInput's ten signed frames-held counters -> the packed 12-bit Inputs the
// replay/netplay path moves around.
//
// Read from the CharacterManager (keyMap, +0x754) rather than from
// KeymapManager::inKeys, because inKeys is only populated when readInKeys is
// set -- true for netplay and replay, FALSE for the local versus match we are
// recording. keyMap is the input the engine actually applied, either way.
//
// The bit order is not guessed: it is the same mapping giuroll's get_input()
// uses against the same offset, and it matches BattleKeys field order.
static unsigned short packKeyInput(const SokuLib::KeyInput &k)
{
	SokuLib::Inputs out;

	out.raw = 0;
	out.battle.up    = k.verticalAxis   < 0;
	out.battle.down  = k.verticalAxis   > 0;
	out.battle.left  = k.horizontalAxis < 0;
	out.battle.right = k.horizontalAxis > 0;
	out.battle.A     = k.a > 0;
	out.battle.B     = k.b > 0;
	out.battle.C     = k.c > 0;
	out.battle.dash  = k.d > 0;
	out.battle.AandB = k.changeCard > 0;
	out.battle.BandC = k.spellcard  > 0;
	return out.raw;
}

// One fixed path, deliberately, for now: this exists to drive giuroll's
// rollback verifier over the most recent 2v2, not to ship replays to players.
// Pairing a sidecar with an arbitrary .rep needs the game's replay-SAVE
// address, which SokuLib does not expose; that is worth finding when 4P
// replays become a user-facing feature, and is dead weight before then.
#define SIDECAR_PATH "replay/last4p.dat"

static void sidecarSave()
{
	if (sidecarInputs.empty())
		return;

	std::ofstream s{SIDECAR_PATH, std::ios::binary};

	if (!s)
		return;
	s.write(reinterpret_cast<const char *>(sidecarInputs.data()), sidecarInputs.size() * sizeof(sidecarInputs[0]));
	printf("4PSoku: wrote %zu frames of P3/P4 input to " SIDECAR_PATH "\n", sidecarInputs.size());
}

static void sidecarLoad()
{
	std::ifstream s{SIDECAR_PATH, std::ios::binary};

	sidecarInputs.clear();
	sidecarCursor = 0;
	if (!s) {
		printf("4PSoku: no " SIDECAR_PATH "; P3/P4 will stand still in this replay.\n");
		return;
	}

	std::array<unsigned short, 2> e;

	while (s.read(reinterpret_cast<char *>(&e), sizeof(e)))
		sidecarInputs.push_back(e);
	printf("4PSoku: loaded %zu frames of P3/P4 input\n", sidecarInputs.size());
}

unsigned displayInputs = 0;
static SokuLib::DrawUtils::Sprite inputsSheet;
static SokuLib::DrawUtils::Sprite numbersSheet;

const auto FUN_00438ce0 = reinterpret_cast<void (__thiscall *)(void *, unsigned, float, float, unsigned, unsigned)>(0x438CE0);
const auto sokuRand = (int (*)(int max))0x4099F0;
const auto getCharName = (char *(*)(int))0x43F3F0;
const auto setRenderMode = [](int mode) {
	((void (__thiscall *)(int, int))0x404B80)(0x896B4C, mode);
};
const auto InputHandler_HandleInput = (bool (__thiscall *)(SokuLib::InputHandler &))0x41FBF0;
const auto getChrName = (char *(*)(SokuLib::Character))0x43F3F0;
const auto getInputManager = (SokuLib::KeymapManager *(*)(int index))0x43E040;
const auto getInputManagerIndex = (char (*)(int index))0x43E070;
const auto initInputManagerArray = (SokuLib::KeyManager *(*)(int index, bool))0x43E6A0;

static std::array<std::map<unsigned char, std::vector<Deck>>, 5> loadedDecks;
std::map<unsigned char, std::map<unsigned short, SokuLib::DrawUtils::Sprite>> cardsTextures;
std::map<unsigned, std::vector<unsigned short>> characterSpellCards;
std::map<unsigned, std::array<unsigned short, 20>> defaultDecks;
std::map<unsigned, std::string> names{
	{ SokuLib::CHARACTER_REIMU, "reimu" },
	{ SokuLib::CHARACTER_MARISA, "marisa" },
	{ SokuLib::CHARACTER_SAKUYA, "sakuya" },
	{ SokuLib::CHARACTER_ALICE, "alice" },
	{ SokuLib::CHARACTER_PATCHOULI, "patchouli" },
	{ SokuLib::CHARACTER_YOUMU, "youmu" },
	{ SokuLib::CHARACTER_REMILIA, "remilia" },
	{ SokuLib::CHARACTER_YUYUKO, "yuyuko" },
	{ SokuLib::CHARACTER_YUKARI, "yukari" },
	{ SokuLib::CHARACTER_SUIKA, "suika" },
	{ SokuLib::CHARACTER_REISEN, "udonge" },
	{ SokuLib::CHARACTER_AYA, "aya" },
	{ SokuLib::CHARACTER_KOMACHI, "komachi" },
	{ SokuLib::CHARACTER_IKU, "iku" },
	{ SokuLib::CHARACTER_TENSHI, "tenshi" },
	{ SokuLib::CHARACTER_SANAE, "sanae" },
	{ SokuLib::CHARACTER_CIRNO, "chirno" },
	{ SokuLib::CHARACTER_MEILING, "meirin" },
	{ SokuLib::CHARACTER_UTSUHO, "utsuho" },
	{ SokuLib::CHARACTER_SUWAKO, "suwako" },
};
static std::map<unsigned char, unsigned> nbSkills{
	{ SokuLib::CHARACTER_REIMU, 12 },
	{ SokuLib::CHARACTER_MARISA, 12 },
	{ SokuLib::CHARACTER_SAKUYA, 12 },
	{ SokuLib::CHARACTER_ALICE, 12 },
	{ SokuLib::CHARACTER_PATCHOULI, 15 },
	{ SokuLib::CHARACTER_YOUMU, 12 },
	{ SokuLib::CHARACTER_REMILIA, 12 },
	{ SokuLib::CHARACTER_YUYUKO, 12 },
	{ SokuLib::CHARACTER_YUKARI, 12 },
	{ SokuLib::CHARACTER_SUIKA, 12 },
	{ SokuLib::CHARACTER_REISEN, 12 },
	{ SokuLib::CHARACTER_AYA, 12 },
	{ SokuLib::CHARACTER_KOMACHI, 12 },
	{ SokuLib::CHARACTER_IKU, 12 },
	{ SokuLib::CHARACTER_TENSHI, 12 },
	{ SokuLib::CHARACTER_SANAE, 12 },
	{ SokuLib::CHARACTER_CIRNO, 12 },
	{ SokuLib::CHARACTER_MEILING, 12 },
	{ SokuLib::CHARACTER_UTSUHO, 12 },
	{ SokuLib::CHARACTER_SUWAKO, 12 }
};

// The vtable is set HERE and not only in loadExtraPlayerInputs' VSPLAYER
// branch, because `keys` below points at these two from static init and the
// engine can dereference them before that branch has ever run.
//
// It calls keymapManager->vtable[1] at 0x46C8FB with no null check:
//     mov edi, [eax]      ; KeyManager -> keymapManager
//     mov eax, [edi]      ; keymapManager -> vtable
//     mov edx, [eax + 4]  ; vtable[1]   <-- faults when vtable is null
// Default construction left the vtable null, so anything reaching the input
// copy before the first VSPLAYER frame crashed reading 0x00000004. That was
// latent: the VSPLAYER branch used to run during the menus and quietly fix it
// up before any replay started. Adding a replay branch that takes precedence
// exposed it, so the ordering dependency is removed rather than reordered.
static std::pair<SokuLib::KeymapManager, SokuLib::KeymapManager> keymaps{
	{(void *)0x85844C, {}, {}, {}, {}, false},
	{(void *)0x85844C, {}, {}, {}, {}, false}
};
static std::pair<SokuLib::KeyManager, SokuLib::KeyManager> keys{{&keymaps.first}, {&keymaps.second}};
static std::pair<SokuLib::PlayerInfo, SokuLib::PlayerInfo> assists = {
	SokuLib::PlayerInfo{SokuLib::CHARACTER_CIRNO, 0, 0, 0, 0, {}, &keys.first},
	SokuLib::PlayerInfo{SokuLib::CHARACTER_MARISA, 1, 0, 0, 0, {}, &keys.second}
};
static std::pair<int, int> healing{-1, -1};

void loadExtraDatFiles(const char *path)
{
	og_loadDat(path);
	SokuLib::v2::loadDatFile((std::string(modFolder) + "/assets.dat").c_str());
}

void loadSoku2CSV(LPWSTR path)
{
	std::ifstream stream{path};
	std::string line;

	printf("Loading character CSV from %S\n", path);
	if (stream.fail()) {
		printf("%S: %s\n", path, strerror(errno));
		return;
	}
	while (std::getline(stream, line)) {
		std::stringstream str{line};
		unsigned id;
		std::string idStr;
		std::string codeName;
		std::string shortName;
		std::string fullName;
		std::string skills;

		std::getline(str, idStr, ';');
		std::getline(str, codeName, ';');
		std::getline(str, shortName, ';');
		std::getline(str, fullName, ';');
		std::getline(str, skills, '\n');
		if (str.fail()) {
			printf("Skipping line %s: Stream failed\n", line.c_str());
			continue;
		}
		try {
			id = std::stoi(idStr);
		} catch (...){
			printf("Skipping line %s: Invalid id\n", line.c_str());
			continue;
		}
		names[id] = codeName;
		nbSkills[id] = (std::count(skills.begin(), skills.end(), ',') + 1 - skills.empty()) * 3;
		printf("%s has %i skills\n", codeName.c_str(), nbSkills[id]);
	}
}

void loadSoku2Config()
{
	puts("Looking for Soku2 config...");

	int argc;
	wchar_t app_path[MAX_PATH];
	wchar_t setting_path[MAX_PATH];
	wchar_t **arg_list = CommandLineToArgvW(GetCommandLineW(), &argc);

	wcsncpy(app_path, arg_list[0], MAX_PATH);
	PathRemoveFileSpecW(app_path);
	if (GetEnvironmentVariableW(L"SWRSTOYS", setting_path, sizeof(setting_path)) <= 0) {
		if (arg_list && argc > 1 && StrStrIW(arg_list[1], L"ini")) {
			wcscpy(setting_path, arg_list[1]);
			LocalFree(arg_list);
		} else {
			wcscpy(setting_path, app_path);
			PathAppendW(setting_path, L"\\SWRSToys.ini");
		}
		if (arg_list) {
			LocalFree(arg_list);
		}
	}
	printf("Config file is %S\n", setting_path);

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];
	GetPrivateProfileStringW(L"Module", nullptr, nullptr, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		wchar_t module_path[MAX_PATH];

		GetPrivateProfileStringW(L"Module", key, nullptr, moduleValue, sizeof(moduleValue), setting_path);

		wchar_t *filename = wcsrchr(moduleValue, '/');

		printf("Check %S\n", moduleValue);
		if (!filename)
			filename = app_path;
		else
			filename++;
		for (int i = 0; filename[i]; i++)
			filename[i] = tolower(filename[i]);
		if (wcscmp(filename, L"soku2.dll") != 0)
			continue;

		hasSoku2 = true;
		wcscpy(module_path, app_path);
		PathAppendW(module_path, moduleValue);
		while (auto result = wcschr(module_path, '/'))
			*result = '\\';
		printf("Soku2 dll is at %S\n", module_path);
		PathRemoveFileSpecW(module_path);
		printf("Found Soku2 module folder at %S\n", module_path);
		PathAppendW(module_path, L"\\config\\info\\characters.csv");
		loadSoku2CSV(module_path);
		return;
	}
}

static void drawBox(const SokuLib::Box &box, const SokuLib::RotationBox *rotation, SokuLib::Color borderColor, SokuLib::Color fillColor)
{
	if (!rotation) {
		SokuLib::DrawUtils::FloatRect rect{};

		rect.x1 = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
		rect.x2 = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
		rect.y1 = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);
		rect.y2 = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);
		rectangle.setRect(rect);
	} else {
		SokuLib::DrawUtils::Rect<SokuLib::Vector2f> rect;

		rect.x1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
		rect.x1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);

		rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x);
		rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt1.y);

		rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x + rotation->pt2.x);
		rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt1.y + rotation->pt2.y);

		rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt2.x);
		rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt2.y);
		rectangle.rawSetRect(rect);
	}

	rectangle.setFillColor(fillColor);
	rectangle.setBorderColor(borderColor);
	rectangle.draw();
}

static void drawCollisionBox(const SokuLib::ObjectManager &manager)
{
	SokuLib::DrawUtils::FloatRect rect{};
	const SokuLib::Box &box = *manager.frameData->collisionBox;

	if (!manager.frameData->collisionBox)
		return;

	rect.x1 = SokuLib::camera.scale * (std::ceil(manager.position.x) + manager.direction * box.left + SokuLib::camera.translate.x);
	rect.x2 = SokuLib::camera.scale * (std::ceil(manager.position.x) + manager.direction * box.right + SokuLib::camera.translate.x);
	rect.y1 = SokuLib::camera.scale * (box.top - std::ceil(manager.position.y) + SokuLib::camera.translate.y);
	rect.y2 = SokuLib::camera.scale * (box.bottom - std::ceil(manager.position.y) + SokuLib::camera.translate.y);

	rectangle.setRect(rect);
	rectangle.setFillColor(SokuLib::Color::Yellow * BOXES_ALPHA);
	rectangle.setBorderColor(SokuLib::Color::Yellow);
	rectangle.draw();
}

static void loadTexture(SokuLib::DrawUtils::Texture &container, const char *path, bool shouldExist = true)
{
	int text = 0;
	SokuLib::Vector2u size;
	int *ret = SokuLib::textureMgr.loadTexture(&text, path, &size.x, &size.y);

	printf("Loading texture %s\n", path);
	if (!ret || !text) {
		puts("Couldn't load texture...");
		if (shouldExist)
			MessageBoxA(SokuLib::window, ("Cannot load game asset " + std::string(path)).c_str(), "Game texture loading failed", MB_ICONWARNING);
	}
	container.setHandle(text, size);
}

static inline void loadTexture(SokuLib::DrawUtils::Sprite &container, const char *path, bool shouldExist = true)
{
	loadTexture(container.texture, path, shouldExist);
}

static void initGuide(Guide &guide)
{
	guide.sprite.setPosition({0, 464});
	guide.sprite.setSize({640, 16});
	guide.sprite.rect.width = guide.sprite.getSize().x;
	guide.sprite.rect.height = guide.sprite.getSize().y;
	guide.sprite.rect.top = 0;
	guide.sprite.rect.left = 0;
}

static void loadCardAssets()
{
	int text;
	char buffer[128];
	SokuLib::DrawUtils::Texture tmp;
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = FONT_HEIGHT;
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 2;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "Tahoma");
	font.create();
	font.setIndirect(desc);
	strcpy(desc.faceName, SokuLib::defaultFontName);
	desc.weight = FW_REGULAR;
	defaultFont.create();
	defaultFont.setIndirect(desc);

	puts("Loading card assets");
	for (int i = 0; i <= 20; i++) {
		sprintf(buffer, "data/card/common/card%03i.bmp", i);
		loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][i], buffer);
	}
	loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][21], "data/battle/cardFaceDown.bmp");
	for (auto &elem : names) {
		auto j = elem.first;

		for (int i = nbSkills[j]; i; i--) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j].c_str(), 99 + i);
			loadTexture(cardsTextures[j][99 + i], buffer);
		}
		for (auto &card : characterSpellCards.at(static_cast<SokuLib::Character>(j))) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j].c_str(), card);
			loadTexture(cardsTextures[j][card], buffer);
		}
	}
	loadTexture(baseSprite,            "data/menu/21_Base.bmp");
	loadTexture(baseSprite,            "data/menu/21_Base.bmp");
	loadTexture(nameSprite,            "data/profile/20_Name.bmp");
	loadTexture(arrowSprite,           "data/profile/deck2/sayuu.bmp");
	loadTexture(noSprite,              "data/menu/23a_No.bmp");
	loadTexture(noSelectedSprite,      "data/menu/23b_No.bmp");
	loadTexture(yesSprite,             "data/menu/22a_Yes.bmp");
	loadTexture(yesSelectedSprite,     "data/menu/22b_Yes.bmp");
	loadTexture(editBoxGuide.sprite,   "data/guide/09.bmp");
	loadTexture(createDeckGuide.sprite,"data/guide/createDeck.bmp");
	loadTexture(selectDeckGuide.sprite,"data/guide/selectDeck.bmp");
	initGuide(createDeckGuide);
	initGuide(selectDeckGuide);
	initGuide(editBoxGuide);
}

static void drawPositionBox(const SokuLib::ObjectManager &manager)
{
	rectangle.setPosition({
		static_cast<int>(SokuLib::camera.scale * (manager.position.x - 2 + SokuLib::camera.translate.x)),
		static_cast<int>(SokuLib::camera.scale * (-manager.position.y - 2 + SokuLib::camera.translate.y))
	});
	rectangle.setSize({
		static_cast<unsigned int>(SokuLib::camera.scale * 5),
		static_cast<unsigned int>(SokuLib::camera.scale * 5)
	});
	rectangle.setFillColor(SokuLib::Color::White);
	rectangle.setBorderColor(SokuLib::Color::White + SokuLib::Color::Black);
	rectangle.draw();
}

static void drawHurtBoxes(const SokuLib::ObjectManager &manager)
{
	if (manager.hurtBoxCount > 5)
		return;

	for (int i = 0; i < manager.hurtBoxCount; i++)
		drawBox(
			manager.hurtBoxes[i],
			manager.hurtBoxesRotation[i],
			SokuLib::Color::Green,
			(manager.frameData->frameFlags.chOnHit ? SokuLib::Color::Cyan : SokuLib::Color::Green) * BOXES_ALPHA
		);
}

static void drawHitBoxes(const SokuLib::ObjectManager &manager)
{
	if (manager.hitBoxCount > 5)
		return;

	for (int i = 0; i < manager.hitBoxCount; i++)
		drawBox(manager.hitBoxes[i], manager.hitBoxesRotation[i], SokuLib::Color::Red, SokuLib::Color::Red * BOXES_ALPHA);
}

static void drawPlayerBoxes(const SokuLib::CharacterManager &manager, bool playerBoxes = true)
{
	if (playerBoxes) {
		drawCollisionBox(manager.objectBase);
		drawHurtBoxes(manager.objectBase);
		drawHitBoxes(manager.objectBase);
		drawPositionBox(manager.objectBase);
	}

	auto array = manager.objects.list.vector();

	for (const auto _elem : array) {
		auto elem = reinterpret_cast<const SokuLib::ProjectileManager *>(_elem);

		if ((elem->isActive && elem->objectBase.hitCount > 0) || elem->objectBase.frameData->attackFlags.value > 0) {
			drawHurtBoxes(elem->objectBase);
			drawHitBoxes(elem->objectBase);
			drawPositionBox(elem->objectBase);
		}
	}
}

// The "dead" profile-name cheat, and why it must not run in netplay.
//
// profiles[] is this machine's OWN profile1/profile2/profile3p/profile4p, read
// straight off disk at startup, and nothing syncs it. In a 4P session all four
// machines evaluate this against four different sets of strings and agree only
// because everyone happens to answer no. One player with a profile named "dead"
// makes their machine kill that seat every frame while nobody else does.
//
// Under rollback that is worse than a desync, it is an unrecoverable one: each
// machine re-simulates the corrected frames using its own answer, so the
// correction never converges. It would present as an unexplainable divergence
// rather than as a cheat somebody switched on.
//
// Replays keep the cheat, deliberately: a replay carries the mainMode of the
// match it recorded, so a local match played with it on still plays back with
// it on, and a netplay replay still plays back with it off.
static bool deadProfileCheat(int player)
{
	if (
		SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER ||
		SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT
	)
		return false;
	return strcmp(profiles[player]->name, "dead") == 0;
}

static void checkHealing(int i, int &healing)
{
	auto hp1 = dataMgr->players[i]->objectBase.hp;
	auto hp2 = dataMgr->players[i + 2]->objectBase.hp;

	if (hp1 == 0 || hp2 == 0) {
		auto dead = dataMgr->players[i + (hp2 == 0 ? 2 : 0)];
		auto dead2 = reinterpret_cast<SokuLib::v2::Player *>(dead);
		auto pos1 = dataMgr->players[i]->objectBase.position;
		auto pos2 = dataMgr->players[i + 2]->objectBase.position;
		auto diffX = pos1.x - pos2.x;
		auto diffY = pos1.y - pos2.y;
		auto &hud = hp1 == 0 ? *(CInfoManager *)0x8985E8 : hud2;

		if (diffX * diffX + diffY * diffY <= HEAL_RADIUS * HEAL_RADIUS && !deadProfileCheat(i + (hp2 == 0 ? 2 : 0)))
			healing++;
		(&hud.p1State)[i].lastHp = healing * dead2->MaxHP / HEALING_TIME;
		dead2->redHP = (&hud.p1State)[i].lastHp;
		if (healing == HEALING_TIME) {
			dead->objectBase.hp = *(unsigned short *) &dead->objectBase.offset_0x186 * HEALTH_PERCENT;
			healing = 0;
			FUN_00438ce0(dead, 0x8B, dead->objectBase.position.x, dead->objectBase.position.y, 1, 1);
			dead->dropInvulTimeLeft = max(dead->dropInvulTimeLeft, HEALING_INVUL);
			//revived = true;
		}
	}
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	auto players = (SokuLib::CharacterManager**)((int)This + 0x0C);

	if (This->matchState == -1)
		return (This->*s_originalBattleMgrOnProcess)();
	//if (players[0]->objectBase.hp == 0 && players[2]->objectBase.hp == 0 || players[1]->objectBase.hp == 0 && players[3]->objectBase.hp == 0)
	//	return SokuLib::SCENE_SELECT;
	if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) {
		if (SokuLib::checkKeyOneshot(DIK_F4, false, false, false))
			disp = !disp;
	} else {
		if (SokuLib::checkKeyOneshot(DIK_F1, false, false, false))
			displayInputs = 1;
		if (SokuLib::checkKeyOneshot(DIK_F2, false, false, false))
			displayInputs = 2;
		if (SokuLib::checkKeyOneshot(DIK_F3, false, false, false))
			displayInputs = 3;
		if (SokuLib::checkKeyOneshot(DIK_F4, false, false, false))
			displayInputs = 4;
		if (SokuLib::checkKeyOneshot(DIK_F5, false, false, false))
			displayInputs = 0;
	}
	if (!init) {
		puts("Init assisters");
		auto alloc = SokuLib::New<void *>(4);
		alloc[0] = dataMgr->activePlayers[0];
		alloc[1] = dataMgr->activePlayers[1];
		alloc[2] = dataMgr->players[2];
		alloc[3] = dataMgr->players[3];
		SokuLib::DeleteFct(*(void **)(*(int *)SokuLib::ADDR_GAME_DATA_MANAGER + 0x40));
		*(void **)(*(int *)SokuLib::ADDR_GAME_DATA_MANAGER + 0x40) = alloc;
		*(void **)(*(int *)SokuLib::ADDR_GAME_DATA_MANAGER + 0x44) = &alloc[4];
		// activePlayers is a three-pointer vector: begin, last, end-of-storage.
		// Only the first two were being updated, so +0x48 kept pointing into
		// the allocation just freed on the line above -- a dangling capacity.
		//
		// Normal play never notices, because the game walks begin..last. Any
		// consumer that reads CAPACITY does: giuroll's savestate does exactly
		// that (read_vec at gameDataManager+0x40, then read_addr(start,
		// end - start)) and dies with "size too big 410296" -- a byte count
		// computed from the stale pointer minus the new allocation.
		//
		// We allocate exactly 4 and never grow, so end-of-storage == last.
		*(void **)(*(int *)SokuLib::ADDR_GAME_DATA_MANAGER + 0x48) = &alloc[4];

		dataMgr->players[2]->objectBase.opponent = &This->rightCharacterManager;
		dataMgr->players[3]->objectBase.opponent = &This->leftCharacterManager;
		for (int i = 0; i < 4; i++)
			dataMgr->players[i]->objectBase.offset_0x14E[0] = i;
		init = true;
	}

	// 4P replay sidecar, stepped once per battle frame.
	//
	// ORDERING MATTERS AND IS THE EASY THING TO GET WRONG. On playback,
	// loadExtraPlayerInputs has already published sidecarInputs[sidecarCursor]
	// into replayExtraInputs for THIS frame, so the cursor is advanced here,
	// after the frame has consumed it. On recording, keyMap holds the input the
	// engine just applied to this frame, so appending here captures frame N's
	// input at frame N. The two stay in phase only because both happen at the
	// same point in the frame -- moving either one is a silent one-frame skew,
	// which looks like a desync rather than like a bug.
	if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY) {
		if (!sidecarLoaded) {
			sidecarLoad();
			sidecarLoaded = true;
		}
		sidecarCursor++;
	} else {
		if (sidecarLoaded) {
			// Left a replay; the next recording starts clean.
			sidecarInputs.clear();
			sidecarCursor = 0;
			sidecarLoaded = false;
		}
		// Record LOCAL versus only, for two separate reasons.
		//
		// Rollback re-runs this function for every frame it re-simulates, so a
		// push_back here would append the same frame once per rollback: the
		// recording would be neither one-entry-per-frame nor in order, and it
		// would grow at whatever rate the connection happens to mispredict.
		//
		// And the sidecar exists to drive giuroll's verifier over a 2v2. A
		// local match does that. Having four machines each write their own copy
		// of the same live match buys nothing and costs a disk write every five
		// seconds during the one activity most sensitive to a frame spike.
		if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSPLAYER) {
			sidecarInputs.push_back({
				packKeyInput(dataMgr->players[2]->keyMap),
				packKeyInput(dataMgr->players[3]->keyMap)
			});
			// Flushed periodically rather than at match end: the replay-save
			// site is not hooked, and a crash or an alt-F4 mid-match would
			// otherwise lose the recording we were about to verify against.
			if (sidecarInputs.size() % 300 == 0)
				sidecarSave();
		}
	}

	for (auto &player : dataMgr->players)
		if (player->keyMap.pause == 1)
			return (This->*s_originalBattleMgrOnProcess)();

	unsigned short hps[4];

	for (int i = 0; i < 4; i++) {
		hps[i] = dataMgr->players[i]->objectBase.hp;
		if (!deadProfileCheat(i))
			continue;
		if (dataMgr->players[i]->objectBase.hp > 0) {
			dataMgr->players[i]->objectBase.hp = 0;
			dataMgr->players[i]->objectBase.action = SokuLib::ACTION_KNOCKED_DOWN_STATIC;
			dataMgr->players[i]->objectBase.animate();
		}
	}
	checkHealing(0, healing.first);
	checkHealing(1, healing.second);
#if 0
	if (revived) {
		for (auto &player : dataMgr->players) {
			FUN_00438ce0(player, 0x8B, player->objectBase.position.x, player->objectBase.position.y, 1, 1);
			player->dropInvulTimeLeft = max(player->dropInvulTimeLeft, HEALING_INVUL);
		}
		revived = false;
	}
#endif
	healingCircle[0].setRotation(fmod(healingCircle[0].getRotation() + 0.01, M_PI * 2));
	healingCircle[1].setRotation(-healingCircle[0].getRotation());

	int ret = (This->*s_originalBattleMgrOnProcess)();
	int index = 0;

	if (This->matchState == 2)
		for (int i = 0; i < 4; i++) {
			if (hps[i] != 0 && dataMgr->players[i]->objectBase.hp == 0) {
				for (int j = (i & 1) ^ 1; j < 4; j += 2)
					if (dataMgr->players[j]->objectBase.opponent == dataMgr->players[i])
						dataMgr->players[j]->objectBase.opponent = dataMgr->players[i ^ 2];
				SokuLib::playSEWaveBuffer(SokuLib::SFX_KNOCK_OUT);
			}
			hps[i] = dataMgr->players[i]->objectBase.hp;
		}
	if (This->matchState <= 2 || This->matchState == 4)
		for (; index < 4; index++) {
			SokuLib::camera.p1X = &dataMgr->players[index]->objectBase.position.x;
			SokuLib::camera.p2X = &dataMgr->players[index]->objectBase.position.x;
			SokuLib::camera.p1Y = &dataMgr->players[index]->objectBase.position.y;
			SokuLib::camera.p2Y = &dataMgr->players[index]->objectBase.position.y;
			if (dataMgr->players[index]->objectBase.hp != 0)
				break;
		}
	for (; index < 4; index++) {
		if (dataMgr->players[index]->objectBase.hp == 0 && (This->matchState <= 2 || This->matchState == 4))
			continue;
		if (*SokuLib::camera.p1X > dataMgr->players[index]->objectBase.position.x)
			SokuLib::camera.p1X = &dataMgr->players[index]->objectBase.position.x;
		if (*SokuLib::camera.p2X < dataMgr->players[index]->objectBase.position.x)
			SokuLib::camera.p2X = &dataMgr->players[index]->objectBase.position.x;
		if (*SokuLib::camera.p1Y > dataMgr->players[index]->objectBase.position.y)
			SokuLib::camera.p1Y = &dataMgr->players[index]->objectBase.position.y;
		if (*SokuLib::camera.p2Y < dataMgr->players[index]->objectBase.position.y)
			SokuLib::camera.p2Y = &dataMgr->players[index]->objectBase.position.y;
	}
	for (int i = 0; i < 4; i++) {
		if (dataMgr->players[i]->keyManager->keymapManager->input.select == 1) {
			int j = (i % 2) ^ 1;

			if (dataMgr->players[i]->objectBase.opponent == dataMgr->players[j])
				j += 2;
			dataMgr->players[i]->objectBase.opponent = dataMgr->players[j];
		}
	}
	if (!SokuLib::menuManager.empty() && SokuLib::sceneId == SokuLib::SCENE_BATTLE)
		return ret;

	if (players[2]->timeStop > 1)
		players[0]->timeStop = max(players[0]->timeStop + 1, 2);
	else if (players[0]->timeStop > 1)
		players[2]->timeStop = max(players[2]->timeStop + 1, 2);
	if (players[3]->timeStop > 1)
		players[1]->timeStop = max(players[1]->timeStop + 1, 2);
	else if (players[1]->timeStop > 1)
		players[3]->timeStop = max(players[3]->timeStop + 1, 2);

	if (This->matchState == 3) {
		players[0]->kdAnimationFinished = players[0]->kdAnimationFinished || players[2]->kdAnimationFinished;
		players[1]->kdAnimationFinished = players[1]->kdAnimationFinished || players[3]->kdAnimationFinished;
	} else {
		players[0]->kdAnimationFinished = players[0]->kdAnimationFinished && players[2]->kdAnimationFinished;
		players[1]->kdAnimationFinished = players[1]->kdAnimationFinished && players[3]->kdAnimationFinished;
	}
	players[0]->roundLost = players[0]->roundLost && players[2]->roundLost;
	players[1]->roundLost = players[1]->roundLost && players[3]->roundLost;
	players[2]->roundLost = players[0]->roundLost;
	players[3]->roundLost = players[1]->roundLost;
	players[2]->kdAnimationFinished = players[0]->kdAnimationFinished;
	players[3]->kdAnimationFinished = players[1]->kdAnimationFinished;
	players[0]->score = max(players[0]->score, players[2]->score);
	players[1]->score = max(players[1]->score, players[3]->score);
	players[2]->score = max(players[0]->score, players[2]->score);
	players[3]->score = max(players[1]->score, players[3]->score);
	return ret;
}

unsigned short getRandomCard(const std::vector<unsigned short> &list, const std::map<unsigned short, unsigned char> &other)
{
	unsigned short card;

	try {
		do
			card = list[sokuRand(list.size())];
		while (other.at(card) >= 4);
	} catch (std::out_of_range &) {}
	return card;
}

void generateFakeDeck(SokuLib::Character chr, SokuLib::Character lastChr, const std::array<unsigned short, 20> *base, std::unique_ptr<std::array<unsigned short, 20>> &buffer)
{
	if (!base) {
		buffer.reset();
		return;
	}

	unsigned last = 100 + nbSkills[chr];
	std::map<unsigned short, unsigned char> used;
	unsigned char c = 0;
	int index = 0;

	buffer = std::make_unique<std::array<unsigned short, 20>>();
	for (int i = 0; i < 20; i++)
		if ((*base)[i] == 21)
			c++;
		else {
			(*buffer)[index] = (*base)[i];
			if (used.find((*buffer)[index]) == used.end())
				used[(*buffer)[index]] = 1;
			else
				used[(*buffer)[index]]++;
			index++;
		}
	if (!c)
		return;

	std::vector<unsigned short> cards;

	for (int i = 0; i < 21; i++)
		cards.push_back(i);
	for (int i = 100; i < last; i++)
		cards.push_back(i);
	for (auto &card : characterSpellCards[chr])
		cards.push_back(card);

	while (c--) {
		(*buffer)[19 - c] = getRandomCard(cards, used);
		if (used.find((*buffer)[19 - c]) == used.end())
			used[(*buffer)[19 - c]] = 1;
		else
			used[(*buffer)[19 - c]]++;
	}
}

void generateFakeDeck(SokuLib::Character chr, SokuLib::Character lastChr, unsigned id, const std::vector<Deck> &bases, std::unique_ptr<std::array<unsigned short, 20>> &buffer)
{
	std::array<unsigned short, 20> randomDeck{21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21};

	if (lastChr == SokuLib::CHARACTER_RANDOM) {
		if (bases.empty())
			return generateFakeDeck(chr, chr, &defaultDecks[chr], buffer);
		return generateFakeDeck(chr, lastChr, &bases[sokuRand(bases.size())].cards, buffer);
	}
	if (id == bases.size())
		return generateFakeDeck(chr, lastChr, &defaultDecks[chr], buffer);
	if (id == bases.size() + 1)
		return generateFakeDeck(chr, lastChr, nullptr, buffer);
	if (id == bases.size() + 2)
		return generateFakeDeck(chr, lastChr, &randomDeck, buffer);
	if (id == bases.size() + 3)
		return generateFakeDeck(chr, lastChr, &bases[sokuRand(bases.size())].cards, buffer);
	return generateFakeDeck(chr, lastChr, &bases[id].cards, buffer);
}

void convertDeckToSokuFormat(const std::unique_ptr<std::array<unsigned short, 20>> &tmp, SokuLib::Dequeue<unsigned short> &buffer)
{
	buffer.clear();
	if (!tmp)
		return;
	for (auto i : *tmp)
		buffer.push_back(i);
}

void generateFakeDecks()
{
	if (generated)
		return;
	generated = true;

	std::unique_ptr<std::array<unsigned short, 20>> tmp;

	if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) {
		generateFakeDeck(SokuLib::leftChar, lastChrs[0], selectedDecks[0], loadedDecks[0][SokuLib::leftChar], tmp);
		convertDeckToSokuFormat(tmp, SokuLib::leftPlayerInfo.effectiveDeck);
		generateFakeDeck(SokuLib::rightChar, lastChrs[1], selectedDecks[1], loadedDecks[1][SokuLib::rightChar], tmp);
		convertDeckToSokuFormat(tmp, SokuLib::rightPlayerInfo.effectiveDeck);
		generateFakeDeck(assists.first.character, lastChrs[2], chrSelectExtra[0].deckHandler.pos, loadedDecks[2][assists.first.character], tmp);
		convertDeckToSokuFormat(tmp, assists.first.effectiveDeck);
		generateFakeDeck(assists.second.character, lastChrs[3], chrSelectExtra[1].deckHandler.pos, loadedDecks[3][assists.second.character], tmp);
		convertDeckToSokuFormat(tmp, assists.second.effectiveDeck);
	} else if (mySlot == 0) {
		generateFakeDeck(SokuLib::leftChar, lastChrs[0], selectedDecks[0], loadedDecks[0][SokuLib::leftChar], tmp);
		convertDeckToSokuFormat(tmp, SokuLib::leftPlayerInfo.effectiveDeck);
	} else if (mySlot == 1) {
		generateFakeDeck(SokuLib::rightChar, lastChrs[1], selectedDecks[1], loadedDecks[0][SokuLib::rightChar], tmp);
		convertDeckToSokuFormat(tmp, SokuLib::rightPlayerInfo.effectiveDeck);
	} else if (mySlot == 2) {
		generateFakeDeck(assists.first.character, lastChrs[2], chrSelectExtra[0].deckHandler.pos, loadedDecks[0][assists.first.character], tmp);
		convertDeckToSokuFormat(tmp, assists.first.effectiveDeck);
	} else if (mySlot == 3) {
		generateFakeDeck(assists.second.character, lastChrs[3], chrSelectExtra[1].deckHandler.pos, loadedDecks[0][assists.second.character], tmp);
		convertDeckToSokuFormat(tmp, assists.second.effectiveDeck);
	}
}

void selectProcessCommon(SokuLib::Select *This, int ret)
{
	if (This->leftSelectionStage == 3 && This->rightSelectionStage == 3 && chrSelectExtra[0].selectState == 3 && chrSelectExtra[1].selectState == 3) {
		if (counter < 60)
			counter++;
	} else {
		counter = 0;
		generated = false;
	}

	for (int i = 0; i < 2; i++) {
		if ((&This->leftSelectionStage)[i] == 1) {
			auto &decks = loadedDecks[i][(&SokuLib::leftPlayerInfo)[i].character];
			auto input = (&This->leftKeys)[i]->input.horizontalAxis;

			if (input == -1 || (input <= -36 && input % 6 == 0)) {
				SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
				if (selectedDecks[i] == 0)
					selectedDecks[i] = decks.size() + 3 - decks.empty();
				else
					selectedDecks[i]--;
			} else if (input == 1 || (input >= 36 && input % 6 == 0)) {
				SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
				if (selectedDecks[i] == decks.size() + 3 - decks.empty())
					selectedDecks[i] = 0;
				else
					selectedDecks[i]++;
			}
		}
	}

	if (This->leftSelectionStage != 3 || This->rightSelectionStage != 3 || chrSelectExtra[0].selectState != 3 || chrSelectExtra[1].selectState != 3 || counter < 30) {
		if (lastChrs[0] != SokuLib::leftChar)
			selectedDecks[0] = 0;
		lastChrs[0] = SokuLib::leftChar;
		if (lastChrs[1] != SokuLib::rightChar)
			selectedDecks[1] = 0;
		lastChrs[1] = SokuLib::rightChar;
		lastChrs[2] = assists.first.character;
		lastChrs[3] = assists.second.character;
	}
	if (ret == SokuLib::SCENE_TITLE) {
		assists.first.character = SokuLib::CHARACTER_CIRNO;
		assists.second.character = SokuLib::CHARACTER_MARISA;
	}
	for (int i = 0; i < 2; i++) {
		auto &dat = chrSelectExtra[i];
		float offset;
		auto &info = (&assists.first)[i];

		if (info.character != SokuLib::CHARACTER_RANDOM) {
			if (dat.portraitCounter)
				dat.portraitCounter--;
		} else {
			if (dat.portraitCounter < 15)
				dat.portraitCounter++;
		}
		if (dat.cursorCounter)
			dat.cursorCounter--;
		if (dat.charNameCounter)
			dat.charNameCounter--;

		if (dat.cursorCounter <= 0)
			dat.cursor->x1 = This->charPortraitStartX + This->charPortraitSliceWidth * dat.chrHandler.pos;
		else
			dat.cursor->x1 -= (dat.cursor->x1 - (This->charPortraitStartX + This->charPortraitSliceWidth * dat.chrHandler.pos)) / 6;
		if (dat.selectState == 0 || dat.selectState == 3) {
			if (dat.deckIndCounter)
				dat.deckIndCounter--;
		} else {
			if (dat.deckIndCounter < 15)
				dat.deckIndCounter++;
		}
		if (dat.selectState == 0) {
			if (dat.chrCounter < 15)
				dat.chrCounter++;
		} else {
			if (dat.chrCounter)
				dat.chrCounter--;
		}

		if (dat.deckIndCounter <= 0)
			offset = 0;
		else if (dat.deckIndCounter >= 0xF)
			offset = 1;
		else
			offset = (dat.deckIndCounter * dat.deckIndCounter) / (15.f * 15.f);

		if (i == 0)
			dat.colorSelect->y1 = 150 - offset * 150;
		else
			dat.colorSelect->y1 = -150 + offset * 150;
		dat.deckSelect->x1 = -280 + 280 * offset;
		dat.circleSprite.rotation += 0.5;
		if (dat.object)
			dat.object->update();
	}
	if (ret == SokuLib::SCENE_LOADING)
		generateFakeDecks();
}

int __fastcall CSelect_OnProcess(SokuLib::Select *This)
{
	int ret = (This->*s_originalSelectOnProcess)();

	if (!SokuLib::menuManager.empty() && *SokuLib::getMenuObj<int>() == 0x859820) {
		auto obj = SokuLib::getMenuObj<int>();
		auto selected = obj[0x1A];

		if (selected >= 2 && selected <= 3)
			editSelectedProfile = selected - 2;
		else
			editSelectedProfile = 4;
	}
	selectProcessCommon(This, ret);
	return ret;
}

int __fastcall CSelectCL_OnProcess(SokuLib::SelectClient *This)
{
	int ret = (This->*s_originalSelectClientOnProcess)();

	if (characterSelectLocked) {
		This->characterSelectEnded = false;
		SokuLib::leftPlayerInfo.character = SokuLib::CHARACTER_REIMU;
		SokuLib::rightPlayerInfo.character = SokuLib::CHARACTER_SANAE;
		assists.first.character = SokuLib::CHARACTER_CIRNO;
		assists.second.character = SokuLib::CHARACTER_MARISA;

		This->base.chrsSelected = 0;
		This->base.leftSelectionStage = 0;
		This->base.rightSelectionStage = 0;
		chrSelectExtra[0].selectState = 0;
		chrSelectExtra[1].selectState = 0;

		This->base.leftCharInput.pos = This->base.leftCharInput.maxValue / 2 - 1;
		This->base.rightCharInput.pos = This->base.rightCharInput.maxValue / 2;
		chrSelectExtra[0].chrHandler.pos = chrSelectExtra[0].chrHandler.maxValue / 2 + 1;
		chrSelectExtra[1].chrHandler.pos = chrSelectExtra[0].chrHandler.maxValue / 2 - 2;

		This->base.leftPalInput.pos = 0;
		This->base.rightPalInput.pos = 0;
		chrSelectExtra[0].palHandler.pos = 0;
		chrSelectExtra[1].palHandler.pos = 0;

		This->base.leftDeckInput.pos = 0;
		This->base.rightDeckInput.pos = 0;
		chrSelectExtra[0].deckHandler.pos = 0;
		chrSelectExtra[1].deckHandler.pos = 0;

		This->base.leftRandomDeck = false;
		This->base.rightRandomDeck = false;

		if (mySlot == -1) {
			while (slotsTaken[selectedSlot])
				selectedSlot = (selectedSlot + 1) % 4;

			auto input = SokuLib::inputMgrs.input.horizontalAxis;

			if (input == -1 || (input <= -36 && input % 6 == 0)) {
				SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
				do
					if (selectedSlot == 0)
						selectedSlot = 3;
					else
						selectedSlot--;
				while (slotsTaken[selectedSlot]);
			} else if (input == 1 || (input >= 36 && input % 6 == 0)) {
				SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
				do
					selectedSlot = (selectedSlot + 1) % 4;
				while (slotsTaken[selectedSlot]);
			}
			if (SokuLib::inputMgrs.input.a == 1)
				mySlot = selectedSlot;
		} else if (SokuLib::inputMgrs.input.b == 1)
			mySlot = -1;
		if (mySlot != remoteMySlot) {
			PacketPlayerJoinAck packet{PLAYER_JOIN_ACK, mySlot};

			sendPacket(packet);
		}
	}
	selectProcessCommon(&This->base, ret);
	return ret;
}

void __fastcall CBattleManager_HandleCollision(SokuLib::BattleManager* This, int unused, void* object, SokuLib::CharacterManager* character)
{
	auto players = (SokuLib::CharacterManager**)((int)This + 0x0C);
	SokuLib::CharacterManager *assist = nullptr;

	for (int i = 0; i < 4; i++)
		if (character == players[i]) {
			assist = players[((i / 2) ^ 1) * 2 + i % 2];
			break;
		}
	ogBattleMgrHandleCollision(This, unused, object, assist);
	ogBattleMgrHandleCollision(This, unused, object, character);
}

static int weirdRand(int key, int delay)
{
	static std::map<int, std::pair<int, int>> elems;
	auto it = elems.find(key);

	if (it == elems.end() || it->second.first == 0) {
		int v = rand();

		elems[key] = {delay, v};
		return v;
	}
	it->second.first--;
	return it->second.second;
}

void renderDeck(SokuLib::Character chr, unsigned select, const std::vector<Deck> &decks, SokuLib::Vector2i pos, const char *overridingName = nullptr)
{
	std::vector<unsigned short> deck;
	std::string name;
	SokuLib::Vector2i base = pos;

	if (select == decks.size()) {
		name = "Default deck";
		deck.resize(20, 21);
		memcpy(deck.data(), defaultDecks[chr].data(), 40);
	} else if (select == decks.size() + 1) {
		name = "No deck";
		deck.resize(0);
	} else if (select == decks.size() + 2) {
		name = "Randomized deck";
		deck.resize(20, 21);
	} else if (select == decks.size() + 3 && !decks.empty())
		return renderDeck(chr, weirdRand((int)&decks, 3) % decks.size(), decks, pos, "Any deck");
	else if (select <= decks.size()) {
		name = decks[select].name;
		deck = {decks[select].cards.begin(), decks[select].cards.end()};
	}

	if (overridingName)
		name = overridingName;

	if (!deck.empty() && displayCards) {
		for (int i = 0; i < 10; i++) {
			SokuLib::DrawUtils::Sprite &sprite = (deck[i] < 100 ? cardsTextures[SokuLib::CHARACTER_RANDOM][deck[i]] : cardsTextures[chr][deck[i]]);

			sprite.setPosition(pos);
			sprite.setSize({10, 16});
			sprite.rect.top = sprite.rect.width = 0;
			sprite.rect.width = sprite.texture.getSize().x;
			sprite.rect.height = sprite.texture.getSize().y;
			sprite.tint = SokuLib::Color::White;
			sprite.draw();
			pos.x += 10;
		}
		pos.x = base.x;
		pos.y += 16;
		for (int i = 0; i < 10; i++) {
			SokuLib::DrawUtils::Sprite &sprite = (deck[i + 10] < 100 ? cardsTextures[SokuLib::CHARACTER_RANDOM][deck[i + 10]] : cardsTextures[chr][deck[i + 10]]);

			sprite.setPosition(pos);
			sprite.setSize({10, 16});
			sprite.rect.top = sprite.rect.width = 0;
			sprite.rect.width = sprite.texture.getSize().x;
			sprite.rect.height = sprite.texture.getSize().y;
			sprite.tint = SokuLib::Color::White;
			sprite.draw();
			pos.x += 10;
		}
	}
	pos.y = base.y + 32;

	SokuLib::DrawUtils::Sprite sprite;
	int text;
	int width = 0;

	if (!SokuLib::textureMgr.createTextTexture(&text, name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}

	pos.x = base.x + 50 - width / 2;
	sprite.setPosition(pos);
	sprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	sprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	sprite.rect = {0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18};
	sprite.tint = SokuLib::Color::White;
	sprite.draw();
}

int __fastcall CSelect_OnRender(SokuLib::Select *This)
{
	auto ret = (This->*s_originalSelectOnRender)();

	if (This->leftSelectionStage == 1 && SokuLib::leftChar != SokuLib::CHARACTER_RANDOM)
		renderDeck(SokuLib::leftChar, selectedDecks[0], loadedDecks[0][SokuLib::leftChar],  {28, 98});
	if (This->rightSelectionStage == 1 && SokuLib::rightChar != SokuLib::CHARACTER_RANDOM)
		renderDeck(SokuLib::rightChar, selectedDecks[1], loadedDecks[1][SokuLib::rightChar], {28, 384});
	if (chrSelectExtra[0].selectState == 1 && assists.first.character != SokuLib::CHARACTER_RANDOM)
		renderDeck(assists.first.character, chrSelectExtra[0].deckHandler.pos, loadedDecks[2][assists.first.character],  {178, 98});
	if (chrSelectExtra[1].selectState == 1 && assists.second.character != SokuLib::CHARACTER_RANDOM)
		renderDeck(assists.second.character, chrSelectExtra[1].deckHandler.pos, loadedDecks[3][assists.second.character], {178, 384});
	return ret;
}

void onlineChrSelectRenderCommon(SokuLib::Select *This)
{
	if (remoteMySlot < 0) {
		chooseBoxBack.draw();
		chooseBoxText.draw();
		cursors2[selectedSlot].setPosition({
			static_cast<int>(chooseBoxText.getPosition().x + chooseBoxText.getSize().x),
			static_cast<int>(240 - cursors2[selectedSlot].getSize().y / 2)
		});
		cursors2[selectedSlot].draw();
	} else if (characterSelectLocked) {
		chooseBoxBack.draw();
		chooseBoxWaitText.draw();
		cursors2[selectedSlot].setPosition({
			static_cast<int>(chooseBoxWaitText.getPosition().x + chooseBoxWaitText.getSize().x),
			static_cast<int>(240 - cursors2[selectedSlot].getSize().y / 2)
		});
		cursors2[selectedSlot].draw();
	}
	if (remoteMySlot == 0 && This->leftSelectionStage == 1 && SokuLib::leftChar != SokuLib::CHARACTER_RANDOM)
		renderDeck(SokuLib::leftChar, selectedDecks[0], loadedDecks[0][SokuLib::leftChar],  {28, 98});
	if (remoteMySlot == 1 && This->rightSelectionStage == 1 && SokuLib::rightChar != SokuLib::CHARACTER_RANDOM)
		renderDeck(SokuLib::rightChar, selectedDecks[1], loadedDecks[1][SokuLib::rightChar], {28, 384});
	if (remoteMySlot == 2 && chrSelectExtra[0].selectState == 1 && assists.first.character != SokuLib::CHARACTER_RANDOM)
		renderDeck(assists.first.character, chrSelectExtra[0].deckHandler.pos, loadedDecks[2][assists.first.character],  {178, 98});
	if (remoteMySlot == 3 && chrSelectExtra[1].selectState == 1 && assists.second.character != SokuLib::CHARACTER_RANDOM)
		renderDeck(assists.second.character, chrSelectExtra[1].deckHandler.pos, loadedDecks[3][assists.second.character], {178, 384});
}

int __fastcall CSelectCL_OnRender(SokuLib::SelectClient *This)
{
	auto ret = (This->*s_originalSelectClientOnRender)();

	onlineChrSelectRenderCommon(&This->base);
	return ret;
}

void renderInput(SokuLib::Inputs input, unsigned time, SokuLib::Vector2i pos)
{
	int high = time == 0 ? 1 : std::pow(10, std::floor(std::log10(time)));
	auto right = pos.x + 118;

	numbersSheet.rect.top = 128;
	while (high) {
		numbersSheet.rect.left = 1 + (time / high % 10) * 14;
		numbersSheet.setPosition(pos);
		numbersSheet.draw();
		pos.x += numbersSheet.getSize().x;
		high /= 10;
	}
	pos.x = right - 25;

	auto dir = 5 - input.battle.left + input.battle.right - input.battle.down * 3 + input.battle.up * 3;
	char index[] = {7, 1, 6, 2, -1, 3, 4, 0, 5};

	if (dir != 5) {
		inputsSheet.setPosition(pos);
		inputsSheet.rect.left = index[dir - 1] * 32;
		inputsSheet.rect.top = 0;
		inputsSheet.setPosition(pos);
		inputsSheet.draw();
	}
	inputsSheet.rect.top = 32;
	pos.x -= 24;
	if (input.battle.BandC) {
		inputsSheet.setPosition(pos);
		inputsSheet.rect.left = 5 * 32;
		inputsSheet.draw();
		pos.x -= 12;
	}
	if (input.battle.AandB) {
		inputsSheet.setPosition(pos);
		inputsSheet.rect.left = 4 * 32;
		inputsSheet.draw();
		pos.x -= 12;
	}
	if (input.battle.dash) {
		inputsSheet.setPosition(pos);
		inputsSheet.rect.left = 3 * 32;
		inputsSheet.draw();
		pos.x -= 12;
	}
	if (input.battle.C) {
		inputsSheet.setPosition(pos);
		inputsSheet.rect.left = 2 * 32;
		inputsSheet.draw();
		pos.x -= 12;
	}
	if (input.battle.B) {
		inputsSheet.setPosition(pos);
		inputsSheet.rect.left = 1 * 32;
		inputsSheet.draw();
		pos.x -= 12;
	}
	if (input.battle.A) {
		inputsSheet.setPosition(pos);
		inputsSheet.rect.left = 0 * 32;
		inputsSheet.draw();
	}
}

void __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	bool hasRIV = LoadLibraryA("ReplayInputView+") != nullptr;
	bool show = hasRIV ? ((RivControl *)((char *)This + sizeof(*This)))->hitboxes : disp;
	int offsets[] = {19, 15, 4, 28};

	(This->*s_originalBattleMgrOnRender)();
	if (This->matchState >= 2)
		for (int i = 3; i >= 0; i--) {
			auto op = dataMgr->players[i]->objectBase.opponent;

			cursors[i].setPosition((
				SokuLib::camera.translate * SokuLib::camera.scale +
				SokuLib::Vector2f{
					op->objectBase.position.x * SokuLib::camera.scale - offsets[i],
					-(op->objectBase.position.y + 200 + cursors[i].getSize().y) * SokuLib::camera.scale
				}).to<int>()
			);
			cursors[i].draw();
		}
	if (init && This->matchState < 6) {
		if (show && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) {
			if (!hasRIV) {
				drawPlayerBoxes(This->leftCharacterManager);
				drawPlayerBoxes(This->rightCharacterManager);
			}
			drawPlayerBoxes(*dataMgr->players[2]);
			drawPlayerBoxes(*dataMgr->players[3]);
		}
	}
	if (displayInputs) {
		SokuLib::DrawUtils::RectangleShape rect;
		SokuLib::Vector2i pos = {20, 100};

		rect.setPosition(pos);
		rect.setSize({94, 300});
		rect.setBorderColor(SokuLib::Color::Transparent);
		rect.setFillColor(SokuLib::Color{0x80, 0x80, 0x80, 0x80});
		rect.draw();

		rect.setPosition({pos.x + 94, pos.y});
		rect.setSize({26, 300});
		rect.setFillColor(SokuLib::Color{0x20, 0x20, 0x20, 0x80});
		rect.draw();

		if (!replayInputs.empty()) {
			SokuLib::Inputs input = replayInputs[replayInputs.size() - 1][displayInputs - 1];
			unsigned timer = 0;

			pos.x += 2;
			pos.y += 2;

			for (size_t i = replayInputs.size(); i; i--) {
				auto &in = replayInputs[i - 1][displayInputs - 1];

				if (in.raw == input.raw) {
					timer++;
					continue;
				}
				renderInput(input, timer, pos);
				pos.y += 26;
				if (pos.y + 24 > 400)
					break;
				input = in;
				timer = 1;
			}
			if (pos.y + 24 < 400)
				renderInput(input, timer, pos);
		}
	}
}

const char *characterSelectXml = "data/scene/select/character/character_4p.dat";
const char *menuProfileXml = "data/menu/menuProfile_4p.dat";
const char *menuSelectXml = "data/menu/select/select_4p.dat";
const char *effectPath = "data/infoeffect/effect_4p.pat";
const char *effectPath_invisible = "data/infoeffect/effect_4p_invis.pat";
const char *battleUpperPath1 = "data/battle/battleUpper_4p.dat";
const char *battleUnderPath1 = "data/battle/battleUnder_4p.dat";
const char *battleUpperPath2 = "data/battle/battleUpper_4p_2.dat";
const char *battleUnderPath2 = "data/battle/battleUnder_4p_2.dat";

void initHud()
{
	DWORD old;

	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	memset((void *)0x47E9CA, 0x90, 6);
	puts("Construct HUD");
	*(const char **)0x47DEC2 = effectPath_invisible;
	*(const char **)0x47DEE5 = battleUpperPath2;
	*(const char **)0x47DEFD = battleUnderPath2;
	((void (__thiscall *)(CInfoManager *))0x47EAF0)(&hud2);
	*(const char **)0x47DEC2 = effectPath;
	*(const char **)0x47DEE5 = battleUpperPath1;
	*(const char **)0x47DEFD = battleUnderPath1;

	SokuLib::CharacterManager** players = (SokuLib::CharacterManager**)((int)&SokuLib::getBattleMgr() + 0xC);
	auto p1 = players[0];
	auto p2 = players[1];
	auto _p1 = dataMgr->players[0];
	auto _p2 = dataMgr->players[1];
	
	puts("Init HUD");
	players[0] = players[2];
	players[1] = players[3];
	dataMgr->players[0] = dataMgr->players[2];
	dataMgr->players[1] = dataMgr->players[3];
	//*(char *)0x47E29C = 0x14;
	((void (__thiscall *)(CInfoManager *))0x47E260)(&hud2);
	//*(char *)0x47E29C = 0xC;
	players[0] = p1;
	players[1] = p2;
	dataMgr->players[0] = _p1;
	dataMgr->players[1] = _p2;
	hud2.p1Portrait->pos.y -= 54;
	hud2.p2Portrait->pos.y -= 54;
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
}

int oldHud = 0;

void __declspec(naked) getOgHud()
{
	__asm {
		MOV ECX, [oldHud]
		RET
	}
}

void __declspec(naked) saveOldHud()
{
	__asm {
		MOV EAX, 0x8985E8
		MOV EAX, [EAX]
		MOV [oldHud], EAX
		RET
	}
}

void __declspec(naked) restoreOldHud()
{
	__asm {
		MOV EAX, [oldHud]
		PUSH EBX
		MOV EBX, 0x8985E8
		MOV [EBX], EAX
		POP EBX
		RET
	}
}

void __declspec(naked) swapStuff()
{
	__asm {
		CMP EDI, 2
		PUSH EBX
		MOV EBX, 0x8985E8
		JL swap2

		LEA EAX, hud2
		MOV [EBX], EAX
		POP EBX
		RET
	swap2:
		MOV EAX, [oldHud]
		MOV [EBX], EAX
		POP EBX
		RET
	}
}

void __fastcall updateCollisionBoxes(SokuLib::BattleManager *This)
{
	auto fct = (void (__thiscall *)(SokuLib::BattleManager *))0x47B840;
	SokuLib::CharacterManager** players = (SokuLib::CharacterManager**)((int)&SokuLib::getBattleMgr() + 0xC);
	float speeds[4] = {0, 0, 0, 0};

	// P1 - P2
	fct(This);
	speeds[0] += players[0]->additionalSpeed.x;
	speeds[1] += players[1]->additionalSpeed.x;
	// P3 - P2
	players[0] = dataMgr->players[2];
	fct(This);
	speeds[2] += players[0]->additionalSpeed.x;
	speeds[1] += players[1]->additionalSpeed.x;
	// P3 - P4
	players[1] = dataMgr->players[3];
	fct(This);
	speeds[2] += players[0]->additionalSpeed.x;
	speeds[3] += players[1]->additionalSpeed.x;
	// P1 - P4
	players[0] = dataMgr->players[0];
	fct(This);
	speeds[0] += players[0]->additionalSpeed.x;
	speeds[3] += players[1]->additionalSpeed.x;
	players[1] = dataMgr->players[1];
	for (int i = 0; i < 4; i++)
		players[i]->additionalSpeed.x = speeds[i];
}

// Where the loading THREAD got to, written to a file.
//
// This hook is the one part of this mod that runs on the game's asset worker
// (the thread at 0x43e5e0), and so it is the one part that can hang the game
// with nothing else noticing. The netplay loading scene, CLoadingCL at
// 0x4286d0, does nothing but poll *(0x89868c), which that worker clears when it
// finishes; if this call never returns, the screen stays black for ever. By
// then the relay's state machine has already reached FIGHT and is satisfied,
// and every giuroll hook lives inside a battle scene that never starts, so no
// other component has anything to say about it.
//
// A file rather than stdout: giuroll owns the console and fills it with
// per-frame warnings, so these lines scroll away exactly when they matter.
static void loadLog(const char *what)
{
	static bool started = false;
	std::ofstream s{"4psoku-load.log", started ? std::ios::app : std::ios::trunc};

	started = true;
	if (s)
		s << what << std::endl;
}

void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::DeckInfo &deck, int param4, SokuLib::Dequeue<short> &newDeck)
{
	loadLog("loadDeckData: entered");
	if (!spawned || init) {
		if (spawned)
			init = false;
		spawned = true;

		SokuLib::CharacterManager** players = (SokuLib::CharacterManager**)((int)&SokuLib::getBattleMgr() + 0xC);

		puts("Not spawned. Loading both assisters");
		puts("Loading character 1");
		loadLog("loadDeckData: constructing P3");
		((void (__thiscall *)(GameDataManager*, int, SokuLib::PlayerInfo &))0x46da40)(dataMgr, 2, assists.first);
		loadLog("loadDeckData: P3 constructed, calling its loader");
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)dataMgr->players[2] + 0x44))(dataMgr->players[2]);
		players[2] = dataMgr->players[2];
		loadLog("loadDeckData: P3 done");

		puts("Loading character 2");
		loadLog("loadDeckData: constructing P4");
		((void (__thiscall *)(GameDataManager*, int, SokuLib::PlayerInfo &))0x46da40)(dataMgr, 3, assists.second);
		loadLog("loadDeckData: P4 constructed, calling its loader");
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)dataMgr->players[3] + 0x44))(dataMgr->players[3]);
		players[3] = dataMgr->players[3];
		loadLog("loadDeckData: P4 done");

		init = false;
		printf("%p %p\n", dataMgr->players[2], dataMgr->players[3]);
		// Init
		//if (hudInit)
		//	((void (__thiscall *)(CInfoManager *, bool))*hud2.vtable)(&hud2, 0);
		hudInit = true;
		loadLog("loadDeckData: initHud");
		initHud();
		loadLog("loadDeckData: assisters ready");
	}
	loadLog("loadDeckData: calling the original");
	s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);
	loadLog("loadDeckData: returned");
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return true;
}

void __fastcall handlePlayerInputs(SokuLib::CharacterManager *This)
{
	og_handleInputs(This);
	if (dataMgr->players[2])
		og_handleInputs(dataMgr->players[2]);
	if (dataMgr->players[3])
		og_handleInputs(dataMgr->players[3]);
}

int loadExtraPlayerInputs()
{
	int g = og_FUN4098D6();

	// Replay FIRST, and that ordering is the actual bug fix.
	//
	// A replay of a local match keeps mainMode == VSPLAYER and only flips
	// subMode to REPLAY, so the VSPLAYER branch below used to fire during
	// playback and bind P3/P4 to whoever was holding a controller. The two
	// extra players were being driven live while P1/P2 replayed -- which is
	// why 4P replays never worked, and why they failed by diverging rather
	// than by crashing.
	if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY) {
		if (sidecarCursor < sidecarInputs.size()) {
			replayExtraInputs[0].inKeys.raw = sidecarInputs[sidecarCursor][0];
			replayExtraInputs[1].inKeys.raw = sidecarInputs[sidecarCursor][1];
		} else {
			// Ran past the sidecar: hold neutral rather than repeat the last
			// input forever, which would look like a stuck stick.
			replayExtraInputs[0].inKeys.raw = 0;
			replayExtraInputs[1].inKeys.raw = 0;
		}
		keys.first.keymapManager = &replayExtraInputs[0];
		keys.second.keymapManager = &replayExtraInputs[1];
	} else if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSPLAYER) {
		auto key = &keymaps.first;

		for (int i = 0; i < 2; i++) {
			key->vtable = (void *)0x85844C;
			key->bindings.index = getInputManagerIndex(i + 2);
			if (key->bindings.index < 0)
				key->bindings.index = 0xFF;
			memcpy(&key->bindings.up, &((key->bindings.index & 0x80) ? profiles[2 + i]->keyboardBindings : profiles[2 + i]->controllerBindings).up, sizeof(key->bindings) - 4);
			key++;
		}
		keys.first.keymapManager = &keymaps.first;
		keys.second.keymapManager = &keymaps.second;
	} else if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER) {
		keys.first.keymapManager = &extraOnlineInputs[0];
		keys.second.keymapManager = &extraOnlineInputs[1];
	}
	return g;
}

void __fastcall onDeath(SokuLib::CharacterManager *This)
{
	auto players = (SokuLib::CharacterManager**)((int)&SokuLib::getBattleMgr() + 0x0C);
	SokuLib::CharacterManager *p1 = players[0];
	SokuLib::CharacterManager *p2 = players[2];

	if (This != p1 && This != p2) {
		p1 = players[1];
		p2 = players[3];
	}
	p1->roundLost = p1->objectBase.hp == 0 && p2->objectBase.hp == 0;
	p2->roundLost = p1->roundLost;
}

unsigned char __fastcall checkWakeUp(SokuLib::CharacterManager *This)
{
	auto ret = og_advanceFrame(This);

	if (This->objectBase.hp == 0)
		ret = 0;
	return ret;
}

static void *og_switchBattleStateBreakAddr;
unsigned endRoundFct = 0x46B420;

void __declspec(naked) healExtraChrs()
{
	__asm {
		MOV EAX, dword ptr [ESI + 0x14]
		MOV DX, word ptr [EAX + 0x186]
		MOV word ptr [EAX + 0x184],DX
		MOV EAX, dword ptr [ESI + 0x18]
		MOV DX, word ptr [EAX + 0x186]
		MOV word ptr [EAX + 0x184], DX
		MOV ECX, [ESI + 0x14]
		CALL [endRoundFct]
		MOV ECX, [ESI + 0x18]
		CALL [endRoundFct]
		XOR ECX, ECX
		MOV dword ptr [healing], ECX
		MOV dword ptr [healing + 4], ECX
		JMP og_switchBattleStateBreakAddr
	}
}

static int (__thiscall *s_ogHudRender)(void *);

const double yPos = 62;

int __fastcall onHudRender(CInfoManager *This)
{
	setRenderMode(2);
	for (int i = 0; i < 2; i++) {
		auto &h = (&healing.first)[i];
		auto p = dataMgr->players[(dataMgr->players[i]->objectBase.hp == 0 ? 0 : 2) + i];

		if (p->objectBase.hp)
			continue;

		for (auto &circle: healingCircle) {
			auto pos = p->objectBase.position;

			pos.x -= circle.getSize().x / 2;
			pos.y *= -1;
			pos.y -= circle.getSize().y / 2;
			circle.setPosition(pos.to<int>());
			circle.draw();
		}
	}
	setRenderMode(1);
	if (SokuLib::getBattleMgr().matchState <= 5) {
		DWORD old;

		::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		*(char *)0x47D93F = 2;
		*(char *)0x47D978 = 2;
		*(char *)0x47D9A0 = 2;
		*(const double **)0x47D9B0 = &yPos;
		*(char *)0x47DADC = 3;
		*(char *)0x47DB15 = 3;
		*(char *)0x47DB4B = 3;
		*(const double **)0x47DB57 = &yPos;
		s_ogHudRender(&hud2);
		*(char *)0x47D93F = 0;
		*(char *)0x47D978 = 0;
		*(char *)0x47D9A0 = 0;
		*(double **)0x47D9B0 = (double *)0x858EB8;
		*(char *)0x47DADC = 1;
		*(char *)0x47DB15 = 1;
		*(char *)0x47DB4B = 1;
		*(double **)0x47DB57 = (double *)0x858EB8;
		::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	}

	int i = s_ogHudRender(This);

	if (SokuLib::getBattleMgr().matchState <= 5) {
		hud2.p1Portrait->render(0, 0);
		hud2.p2Portrait->render(640, 0);
		(**(void (__thiscall **)(void *, int, int, int))(*(int *)&hud2.offset_0x004[0x94] + 0x18))(&hud2.offset_0x004[0x94], 0, 0, 0x1A);
	}
	return i;
}

void __declspec(naked) updateOtherHud()
{
	__asm {
		LEA ECX, [hud2]
		MOV EAX, 0x47D6F0
		CALL EAX
		RET
	}
}

static int skipHealthRegen = 0x47B092;
static int dontSkipHealthRegen = 0x47B083;
static int extraHealing[2];

void __declspec(naked) setHealthRegen()
{
	__asm {
		MOVSX EAX, byte ptr [ECX + 0x14E]
		MOVSX ECX, BX
		MOV dword ptr [ESP + 0x20], ECX
		CMP AL, 2
		JLE dontSkip
		DEC EAX
		DEC EAX
		LEA EAX, [extraHealing + EAX * 4]
		JMP skipHealthRegen
	dontSkip:
		JMP dontSkipHealthRegen
	}
}

void __declspec(naked) addHealthRegen()
{
	__asm {
		CALL SokuLib::getBattleMgr
		MOV EBX, [EAX + 0x14]
		MOVSX DI, word ptr [EBX + 0x186]
		MOVSX CX, word ptr [EBX + 0x184]
		ADD ECX, dword ptr [extraHealing]
		CMP CX, DI
		JL lowerThanMax1
		MOV CX, DI
	lowerThanMax1:
		CMP CX, 0x0
		JG higherThanZero1
		MOV ECX, 0
	higherThanZero1:
		MOV [EBX + 0x184], CX

		MOV EBX, [EAX + 0x18]
		MOVSX DI, word ptr [EBX + 0x186]
		MOVSX CX, word ptr [EBX + 0x184]
		ADD ECX, dword ptr [extraHealing + 4]
		CMP CX, DI
		JL lowerThanMax2
		MOV CX, DI
	lowerThanMax2:
		CMP CX, 0x0
		JG higherThanZero2
		MOV ECX, 0
	higherThanZero2:
		MOV [EBX + 0x184], CX
		RET
	}
}

void __declspec(naked) clearHealthRegen()
{
	__asm {
		MOV [extraHealing], 0
		MOV [extraHealing + 4], 0
		RET
	}
}

static SokuLib::Select *(__fastcall *og_SelectConstruct)(SokuLib::Select *This);

SokuLib::Select *__fastcall CSelect_construct(SokuLib::Select *This)
{
	og_SelectConstruct(This);

	int ret = 0;
	SokuLib::Vector2i size;
	char buffer[128];

	for (int i = 0; i < 2; i++) {
		auto &dat = chrSelectExtra[i];
		auto &profileInfo = (&assists.first)[i];

		if (dat.isInit) {
			((void (__thiscall *)(CEffectManager  *))0x4241B0)(&dat.effectMgr);
			SokuLib::textureMgr.deallocate(dat.portraitTexture);
			SokuLib::textureMgr.deallocate(dat.circleTexture);
		}
		This->designBase3.getById(&dat.name,        100 + i + 2);
		This->designBase3.getById(&dat.portrait,    900 + i + 2);
		This->designBase3.getById(&dat.spellCircle, 800 + i + 2);
		This->designBase3.getById(&dat.charObject,  700 + i + 2);
		This->designBase3.getById(&dat.deckObject,  720 + i + 2);
		This->designBase3.getById(&dat.profileBack, 300 + i + 2);
		This->designBase3.getById(&dat.gear,        600 + i + 2);
		This->designBase3.getById(&dat.cursor,      400 + i + 2);
		This->designBase3.getById(&dat.deckSelect,  320 + i + 2);
		This->designBase3.getById(&dat.colorSelect, 310 + i + 2);
		dat.cursor->active = true;
		dat.gear->active = true;
		dat.profileBack->active = true;
		dat.deckSelect->active = true;
		dat.colorSelect->active = true;
		// data/scene/select/character/06b_wheel%dp.bmp
		sprintf(buffer, (char *)0x857C18, i + 3);
		if (!SokuLib::textureMgr.loadTexture(&ret, buffer, &size.x, &size.y))
			printf("Failed to load %s\n", buffer);
		dat.gearSprite.setTexture(ret, 0, 0, size.x, size.y, size.x / 2, size.y / 2);
		ret = 0;

		dat.needInit = true;
		if (*reinterpret_cast<SokuLib::NetObject **>(SokuLib::ADDR_PNETOBJECT))
			dat.input = &extraOnlineInputs[i];
		else
			dat.input = nullptr;
		dat.object = nullptr;
		dat.selectState = 0;
		dat.chrHandler.maxValue = 20;
		dat.chrHandler.pos = 0;
		while ((*(unsigned **)&This->offset_0x018[0x84])[dat.chrHandler.pos] != profileInfo.character)
			dat.chrHandler.pos++;
		dat.chrHandler.posCopy = dat.chrHandler.pos;
		dat.palHandler.maxValue = 8;
		dat.palHandler.pos = 0;
		dat.palHandler.posCopy = 0;
		dat.deckHandler.maxValue = 4;
		dat.deckHandler.pos = 0;
		dat.deckHandler.posCopy = 0;
		dat.portraitCounter = 15;
		dat.charNameCounter = 15;
		dat.chrCounter = 15;
		dat.cursorCounter = 60;
		dat.cursor->x1 = 700 * i - 50;
		dat.isInit = true;
		((void (__thiscall *)(CEffectManager  *))0x422CF0)(&dat.effectMgr);
		dat.palHandler.pos = profileInfo.palette;
		dat.deckHandler.pos = profileInfo.deck;
		// data/scene/select/character/09b_character/character_%02d.bmp
		sprintf(buffer, (char *)0x85785C, profileInfo.character);
		if (!SokuLib::textureMgr.loadTexture(&ret, buffer, &size.x, &size.y))
			printf("Failed to load %s\n", buffer);
		dat.portraitSprite.setTexture2(ret, 0, 0, size.x, size.y);
		dat.portraitTexture = ret;
	}
	return This;
}

void __declspec(naked) checkUsedInputs()
{
	__asm {
		MOV EAX, 0
	loop1:
		CMP EAX, ESI
		JE loopEnd
		JG normal
		MOV byte ptr [ESP + 0xC], 0x1
		JMP loop0
	normal:
		MOV byte ptr [ESP + 0xC], 0x0
	loop0:

		MOV CL, [EAX + 0x898678]
		CMP CL, [ESI + 0x898678]
		JNE loopEnd

		CMP byte ptr [ESP + 0xC], 0x0
		JE resetOther

		MOV byte ptr [ESI + 0x898678], 0xFE
		RET

	resetOther:
		MOV byte ptr [EAX + 0x898678], 0xFE
		CMP EAX, 2
		JGE extraData

		MOV [0x898680 + EAX * 4], 0
		JMP loopEnd

	extraData:
		DEC EAX
		DEC EAX
		MOV [chrSelectExtraInputs + EAX * 4], 0
		INC EAX
		INC EAX

	loopEnd:
		INC EAX
		CMP EAX, 4
		JL loop1
		RET
	}
}

void __declspec(naked) setInputPointer()
{
	__asm {
		CMP ESI, 2
		JGE extraData

		MOV [0x898680 + ESI * 4], EAX
		RET

	extraData:
		DEC ESI
		DEC ESI
		MOV [chrSelectExtraInputs + ESI * 4], EAX
		RET
	}
}

void __declspec(naked) cmpInputPointer()
{
	__asm {
		CMP ESI, 2
		JGE extraData

		CMP [0x898680 + ESI * 4], 0
		RET

	extraData:
		PUSH ESI
		DEC ESI
		DEC ESI
		CMP [chrSelectExtraInputs + ESI * 4], 0
		POP ESI
		RET
	}
}

void __declspec(naked) loadInputPointer()
{
	__asm {
		CMP EAX, 2
		JGE extraData

		MOV EAX, [0x898680 + EAX * 4]
		RET

	extraData:
		DEC EAX
		DEC EAX
		MOV EAX, [chrSelectExtraInputs + EAX * 4]
		RET
	}
}

void __fastcall initExtraInputsLight(SokuLib::Select *This, int b)
{
	auto keyboard = (SokuLib::KeymapManager *)0x8986A8;

	for (int i = 2 + b; i; i--) {
		if ((getInputManagerIndex(i - 1) & 0xFE) == 0xFE) {
			if (chrSelectExtra[b].selectState == 0) {
				if (i == 3)
					chrSelectExtra[0].input = keyboard;
				else if (i != 3)
					(&This->leftKeys)[i - 1] = keyboard;
			} else {
				if (i == 3)
					chrSelectExtra[0].input = nullptr;
				else if (i != 3)
					(&This->leftKeys)[i - 1] = nullptr;
			}

			if ((i == 3 && chrSelectExtra[0].selectState == 3) || (i != 3 && (&This->leftSelectionStage)[i - 1] == 3)) {
				chrSelectExtra[b].input = keyboard;
				if (chrSelectExtra[b].input && b == 0)
					initExtraInputsLight(This, b + 1);
			} else
				chrSelectExtra[b].input = nullptr;
			return;
		}
	}
	chrSelectExtra[b].input = nullptr;
}

void __fastcall initExtraInputs(SokuLib::Select *This, int b)
{
	if (!chrSelectExtra[b].needInit) {
		chrSelectExtra[b].input = getInputManager(2 + b);
		if (chrSelectExtra[b].input && b == 0)
			initExtraInputs(This, b + 1);
		return;
	}
	initInputManagerArray(2 + b, false);

	auto ptr = getInputManager(2 + b);

	if (ptr != nullptr) {
		chrSelectExtra[b].input = ptr;
		if (chrSelectExtra[b].input && b == 0)
			initExtraInputs(This, b + 1);
		return;
	}
	initExtraInputsLight(This, b);
}

void __declspec(naked) initExtraInputsLight_hook()
{
	__asm {
		MOV dword ptr [ESI + 0x14], EAX
		TEST EAX, EAX
		JZ ret_

		PUSH EAX
		PUSH EDI
		PUSH ECX
		PUSH EBX
		PUSH EDX
		PUSH ESI
		MOV ECX, ESI
		XOR EDX, EDX
		CALL initExtraInputsLight
		POP ESI
		POP EDX
		POP EBX
		POP ECX
		POP EDI
		POP EAX
	ret_:
		XOR EDI, EDI
		RET
	}
}

void __declspec(naked) initExtraInputs_hook()
{
	__asm {
		MOV dword ptr [ESI + 0x14], EAX
		TEST EAX, EAX
		JZ ret_

		PUSH EAX
		PUSH EDI
		PUSH ECX
		PUSH EBX
		PUSH EDX
		PUSH ESI
		MOV ECX, ESI
		XOR EDX, EDX
		CALL initExtraInputs
		POP ESI
		POP EDX
		POP EBX
		POP ECX
		POP EDI
		POP EAX
	ret_:
		CMP EAX, EDI
		RET
	}
}

void __fastcall renderChrSelectChrData(int index)
{
	auto &dat = chrSelectExtra[index];
	float offset;

	if (dat.portraitCounter <= 0)
		offset = 0;
	else if (dat.portraitCounter >= 0xF)
		offset = 1;
	else
		offset = (dat.portraitCounter * dat.portraitCounter) / (15.f * 15.f);
	dat.portraitSprite.render(offset * 640 + dat.portrait->x2, dat.portrait->y2);
}

void __fastcall renderChrSelectChrDataGear(int index)
{
	auto &dat = chrSelectExtra[index];

	dat.gearSprite.rotation = dat.cursor->x1 * 2.5;
	dat.gearSprite.setColor(0xFF000000);
	dat.gearSprite.render(dat.cursor->x1 + dat.gear->x2 + 2, dat.gear->y2);
	dat.gearSprite.setColor(0xFFFFFFFF);
	dat.gearSprite.render(dat.cursor->x1 + dat.gear->x2, dat.gear->y2);
}

void __fastcall renderChrSelectChrName(SokuLib::Select *This, int index)
{
	auto &dat = chrSelectExtra[index];
	auto &info = (&assists.first)[index];
	float offset;

	if (dat.charNameCounter <= 0)
		offset = 0;
	else if (dat.charNameCounter >= 0xF)
		offset = 1;
	else
		offset = (dat.charNameCounter * dat.charNameCounter) / (15.f * 15.f);

	This->charNameSprites[info.character == SokuLib::CHARACTER_RANDOM ? 20 : info.character].render(offset * 200 + dat.name->x2, dat.name->y2);
	if (dat.object && info.character != SokuLib::CHARACTER_RANDOM) {
		if (dat.chrCounter <= 0)
			offset = 0;
		else if (dat.chrCounter >= 0xF)
			offset = 1;
		else
			offset = (dat.chrCounter * dat.chrCounter) / (15.f * 15.f);
		setRenderMode(2);
		if (index == 0)
			offset *= -1;
		dat.circleSprite.render(dat.spellCircle->x2, dat.spellCircle->y2 + 200 * offset);
		dat.object->position.y = dat.charObject->y2 + offset * 200;
		dat.object->render();
	}
}

void __fastcall renderChrSelectProfile(int index)
{
	auto *netObject = *reinterpret_cast<SokuLib::NetObject **>(SokuLib::ADDR_PNETOBJECT);

	if (netObject == nullptr) {
		profiles[index + 2]->sprite.render(chrSelectExtra[index].profileBack->x2 + 88, chrSelectExtra[index].profileBack->y2 + 10);
		return;
	}
	extraNetProfileNames[index].render(chrSelectExtra[index].profileBack->x2 + 88, chrSelectExtra[index].profileBack->y2 + 10);
}

void __declspec(naked) renderChrSelectChrData_hook()
{
	__asm {
		MOV ECX, [ESP + 0x1C]
		PUSH EAX
		PUSH ESI
		PUSH EDI
		PUSH EBX
		CALL renderChrSelectChrData
		POP EBX
		POP EDI
		POP ESI
		POP EAX
		CMP EAX, -3
		CMP [ESI + 0xFFFFFE4C], 0
		RET
	}
}

void __declspec(naked) renderChrSelectChrDataGear_hook()
{
	__asm {
		MOV ECX, [ESP + 0x1C]
		PUSH EAX
		PUSH ESI
		PUSH EDI
		PUSH EBX
		CALL renderChrSelectChrDataGear
		POP EBX
		POP EDI
		POP ESI
		POP EAX
		MOV ECX, [ESI + 0x148]
		RET
	}
}

void __declspec(naked) renderChrSelectChrName_hook()
{
	__asm {
		MOV ECX, EBX
		MOV EDX, ESI
		PUSH EAX
		PUSH ESI
		PUSH EDI
		PUSH EBX
		CALL renderChrSelectChrName
		POP EBX
		POP EDI
		POP ESI
		POP EAX
		INC ESI
		ADD EDI, 4
		RET
	}
}

void __declspec(naked) renderChrSelectProfile_hook()
{
	__asm {
		MOV ECX, EDI
		PUSH EAX
		PUSH ESI
		PUSH EDI
		PUSH EBX
		CALL renderChrSelectProfile
		POP EBX
		POP EDI
		POP ESI
		POP EAX
		INC EDI
		ADD ESI, 4
		RET
	}
}

void __fastcall changePalette(SokuLib::Select *This, int index, char palette, bool dir)
{
	bool ok = false;
	auto &mine = (index < 2 ? (&SokuLib::leftPlayerInfo)[index] : (&assists.first)[index - 2]);
	auto &mine2 = (index == 0 ? This->leftPalInput : index == 1 ? This->rightPalInput : chrSelectExtra[index - 2].palHandler);

	while (!ok) {
		ok = true;
		for (int i = 0; i < 2; i++) {
			if (index == i)
				continue;
			if ((&SokuLib::leftPlayerInfo)[i].character == mine.character && (&SokuLib::leftPlayerInfo)[i].palette == palette && (&This->leftSelectionStage)[i] != 0) {
				palette = (palette + 7 - dir * 6) % 8;
				ok = false;
			}
		}
		for (int i = 0; i < 2; i++) {
			if (index == i + 2)
				continue;
			if ((&assists.first)[i].character == mine.character && (&assists.first)[i].palette == palette && chrSelectExtra[i].selectState != 0) {
				palette = (palette + 7 - dir * 6) % 8;
				ok = false;
			}
		}
	}
	mine.palette = palette;
	mine2.pos = palette;
	mine2.posCopy = palette;
}

void __declspec(naked) changePalette_hook0()
{
	__asm {
		PUSH EDX
		MOV EAX, [EDX]
		MOV ECX, ESI
		MOV EDX, 1
		PUSH EDX
		PUSH EAX
		MOV EDX, EDI
		SHR EDX, 5
		CALL changePalette
		POP EDX
		RET
	}
}

void __declspec(naked) changePalette_hook1()
{
	__asm {
		PUSH EDX
		MOV EAX, [EDX]
		MOV ECX, ESI
		MOV EDX, [EBX]
		MOV EDX, [EDX + 0x3C]
		CMP EDX, 0
		JGE under
		XOR EDX, EDX
		JMP END
		under:
		MOV EDX, 1
		end:
		PUSH EDX
		PUSH EAX
		MOV EDX, EDI
		SHR EDX, 5
		CALL changePalette
		POP EDX
		RET
	}
}

int updateCharacterSelect_hook_failAddr = 0x42091D;
int updateCharacterSelect_hook_retAddr = 0x4208F4;

void updateCharacterSelect2(SokuLib::Select *This, unsigned i)
{
	auto &dat = chrSelectExtra[i];
	char buffer[128];
	SokuLib::Vector2i size;
	auto &info = (&assists.first)[i];

	if (!dat.input)
		return;
	switch (dat.selectState) {
	case 0:
		dat.chrHandler.axis = &dat.input->input.horizontalAxis;
		if (InputHandler_HandleInput(dat.chrHandler)) {
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
			info.character = *((SokuLib::Character *(__thiscall *)(const void *, int))0x420380)(&This->offset_0x018[0x80], dat.chrHandler.pos);
			sprintf(buffer, (char *)0x85785C, info.character);
			SokuLib::textureMgr.deallocate(dat.portraitTexture);
			if (!SokuLib::textureMgr.loadTexture(&dat.portraitTexture, buffer, &size.x, &size.y))
				printf("Failed to load %s\n", buffer);
			dat.portraitSprite.setTexture2(dat.portraitTexture, 0, 0, size.x, size.y);
			dat.portraitCounter = 15;
			dat.charNameCounter = 15;
			dat.cursorCounter = 30;
			break;
		}
		if (dat.input->input.a == 1) {
			char *name = getCharName(info.character);

			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
			dat.selectState = 1;
			changePalette(This, i + 2, info.palette, true);
			sprintf(buffer, (char *)0x8578EC, info.character);
			SokuLib::textureMgr.deallocate(dat.circleTexture);
			if (!SokuLib::textureMgr.loadTexture(&dat.circleTexture, buffer, &size.x, &size.y))
				printf("Failed to load %s\n", buffer);
			dat.circleSprite.setTexture(dat.circleTexture, 0, 0, size.x, size.y, size.x / 2, size.y / 2);
			((void (__thiscall *)(CEffectManager *))dat.effectMgr.vtable[2])(&dat.effectMgr);
			// data/character/%s/palette%d.bmp
			sprintf(buffer, (char *)0x8578C8, name, info.palette);
			((void (__thiscall *)(int, char *, int, int))0x408BE0)(0x8A0048, buffer, 0x896B88, 0x10);
			// data/scene/select/character/%s/stand.xml
			sprintf(buffer, (char *)0x85789C, name);
			((void (__thiscall *)(CEffectManager *, char *, int))dat.effectMgr.vtable[1])(&dat.effectMgr, buffer, 0);
			dat.object = ((SokuLib::v2::AnimationObject *(__thiscall *)(CEffectManager *, int, float, float, int, int, int))dat.effectMgr.vtable[3])(&dat.effectMgr, 0, dat.charObject->x2, dat.charObject->y2, 1, 0, 0);
			dat.object->setPose(0);
			if (lastChrs[2 + i] != info.character)
				dat.deckHandler.pos = 0;
			dat.deckHandler.maxValue = loadedDecks[2 + i][info.character].size() + 3;
		} else if (dat.input->input.b == 1) {
		} else if (dat.input->input.d == 1) {
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
			dat.selectState = 3;
			info.character = SokuLib::CHARACTER_RANDOM;
			dat.charNameCounter = 15;
		}
		break;
	case 1:
		dat.palHandler.axis = &dat.input->input.verticalAxis;
		dat.deckHandler.axis = &dat.input->input.horizontalAxis;
		if (InputHandler_HandleInput(dat.deckHandler))
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
		if (InputHandler_HandleInput(dat.palHandler)) {
			char *name = getCharName(info.character);
			int poseId = dat.object->frameState.poseId;
			int poseFrame = dat.object->frameState.poseFrame;

			changePalette(This, i + 2, dat.palHandler.pos, *dat.palHandler.axis > 0);
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
			((void (__thiscall *)(CEffectManager *))dat.effectMgr.vtable[2])(&dat.effectMgr);
			// data/character/%s/palette%d.bmp
			sprintf(buffer, (char *)0x8578C8, name, info.palette);
			((void (__thiscall *)(int, char *, int, int))0x408BE0)(0x8A0048, buffer, 0x896B88, 0x10);
			// data/scene/select/character/%s/stand.xml
			sprintf(buffer, (char *)0x85789C, name);
			((void (__thiscall *)(CEffectManager *, char *, int))dat.effectMgr.vtable[1])(&dat.effectMgr, buffer, 0);
			dat.object = ((SokuLib::v2::AnimationObject *(__thiscall *)(CEffectManager *, int, float, float, int, int, int))dat.effectMgr.vtable[3])(&dat.effectMgr, 0, dat.charObject->x2, dat.charObject->y2, 1, 0, 0);
			dat.object->setPose(poseId);
			dat.object->frameState.poseFrame = poseFrame;
		}
		if (dat.input->input.a == 1) {
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
			dat.object->setAction(1);
			dat.selectState = 3;
		} else if (dat.input->input.b == 1) {
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CANCEL);
			dat.selectState = 0;
		}
	case 3:
		if (dat.input->input.b == 1) {
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CANCEL);
			dat.selectState = 0;
			if (info.character == SokuLib::CHARACTER_RANDOM) {
				info.character = *((SokuLib::Character *(__thiscall *)(const void *, int))0x420380)(&This->offset_0x018[0x80], dat.chrHandler.pos);
				dat.charNameCounter = 15;
			}
		}
	}
}

bool updateCharacterSelect(SokuLib::Select *This)
{
	updateCharacterSelect2(This, 0U);
	updateCharacterSelect2(This, 1U);
	return chrSelectExtra[0].selectState == 3 && chrSelectExtra[1].selectState == 3;
}

void __declspec(naked) updateCharacterSelect_hook()
{
	__asm {
		PUSH ESI
		CALL updateCharacterSelect
		POP ESI
		TEST AL, AL
		JNZ ok
		JMP updateCharacterSelect_hook_failAddr
	ok:
		CMP byte ptr [ESI + 0x22C0], 0x3
		JMP updateCharacterSelect_hook_retAddr
	}
}

void onChrSelectComplete()
{
	if (assists.first.character == SokuLib::CHARACTER_RANDOM)
		assists.first.character = static_cast<SokuLib::Character>(sokuRand(20));
	if (assists.second.character == SokuLib::CHARACTER_RANDOM)
		assists.second.character = static_cast<SokuLib::Character>(sokuRand(20));
}

void __fastcall onStageSelectCancel(SokuLib::Select *This, int index)
{
	if ((&assists.first)[index].character == SokuLib::CHARACTER_RANDOM)
		(&assists.first)[index].character = (*(SokuLib::Character **)&This->offset_0x018[0x84])[chrSelectExtra[index].chrHandler.pos];
	chrSelectExtra[index].selectState = 0;
	chrSelectExtra[index].input = chrSelectExtraInputs[index];
}

void __declspec(naked) onStageSelectCancel_hook()
{
	__asm {
		MOV EDX, EBP
		MOV ECX, ESI
		JMP onStageSelectCancel
	}
}

void onCharacterSelectInit()
{
	if (*reinterpret_cast<SokuLib::NetObject **>(SokuLib::ADDR_PNETOBJECT)) {
		chrSelectExtraInputs[0] = &extraOnlineInputs[0];
		chrSelectExtraInputs[1] = &extraOnlineInputs[1];
	} else {
		chrSelectExtraInputs[0] = nullptr;
		chrSelectExtraInputs[1] = nullptr;
	}
}

int __fastcall CTitle_OnProcess(SokuLib::Title *This)
{
	if (SokuLib::menuManager.empty() || *SokuLib::getMenuObj<int>() != SokuLib::ADDR_VTBL_DECK_CONSTRUCTION_CHR_SELECT_MENU)
		editSelectedProfile = 4;
	return (This->*s_originalTitleOnProcess)();
}

void updateGuide(Guide &guide)
{
	if (guide.active && guide.alpha != 255)
		guide.alpha += 15;
	else if (!guide.active && guide.alpha)
		guide.alpha -= 15;
}

void renderGuide(Guide &guide)
{
	guide.sprite.tint.a = guide.alpha;
	guide.sprite.draw();
}

bool saveDeckFromGame(SokuLib::ProfileDeckEdit *This, std::array<unsigned short, 20> &deck)
{
	unsigned index = 0;

	for (auto &pair : *This->editedDeck) {
		auto i = pair.second;

		while (i) {
			if (index == 20)
				return false;
			deck[index] = pair.first;
			i--;
			index++;
		}
	}
	while (index < 20) {
		deck[index] = 21;
		index++;
	}
	std::sort(deck.begin(), deck.end());
	return true;
}

void loadDeckToGame(SokuLib::ProfileDeckEdit *This, const std::array<unsigned short, 20> &deck)
{
	int count = 0;

	This->editedDeck->clear();
	for (int i = 0; i < 20; i++)
		if (deck[i] != 21) {
			auto iter = This->editedDeck->find(deck[i]);
			if (iter == This->editedDeck->end())
				(*This->editedDeck)[deck[i]] = 1;
			else
				iter->second++;
			count++;
		}
	This->displayedNumberOfCards = count;
}

void __fastcall CProfileDeckEdit_SwitchCurrentDeck(SokuLib::ProfileDeckEdit *This, int, int DeckID)
{
	auto FUN_0044f930 = SokuLib::union_cast<void (SokuLib::ProfileDeckEdit::*)(char param_1)>(0x44F930);

	This->selectedDeck = 0;
	if (!saveDeckFromGame(This, editedDecks[editSelectedDeck].cards)) {
		errorCounter = 120;
		return;
	}
	if (DeckID == 1) {
		if (editSelectedDeck == editedDecks.size() - 1)
			editSelectedDeck = 0;
		else
			editSelectedDeck++;
	} else {
		if (editSelectedDeck == 0)
			editSelectedDeck = editedDecks.size() - 1;
		else
			editSelectedDeck--;
	}
	loadDeckToGame(This, editedDecks[editSelectedDeck].cards);
	(This->*FUN_0044f930)('\0');
}

void renameBoxRender()
{
	int text;
	int width;
	SokuLib::DrawUtils::Sprite textSprite;
	SokuLib::DrawUtils::RectangleShape rect;

	nameSprite.setPosition({160, 192});
	nameSprite.setSize(nameSprite.texture.getSize());
	nameSprite.rect = {
		0, 0,
		static_cast<int>(nameSprite.texture.getSize().x),
		static_cast<int>(nameSprite.texture.getSize().y)
	};
	nameSprite.tint = SokuLib::Color::White;
	nameSprite.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, nameBuffer, font, TEXTURE_SIZE, 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	auto render = (int(__thiscall*) (void*, float, float))0x42a050;

	render(editingBoxObject, 276, 217);
}

void openRenameBox(SokuLib::ProfileDeckEdit *This)
{
	auto setup_global = (int(__thiscall*) (void*, bool))0x40ea10;
	auto init_fun = (void(__thiscall*) (void*, int, int))0x429e70;

	SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
	if (editSelectedDeck == editedDecks.size() - 1) {
		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}

	//:magic_wand:
	init_fun(editingBoxObject, 0x89A4F8, 24);
	setup_global((void*)0x8A02F0, true);
	strncpy((char *)0x8A02F8, editedDecks[editSelectedDeck].name.c_str(), 24);
	renameBoxDisplayed = true;
}

void renameBoxUpdate(SokuLib::KeyManager *keys)
{
	auto update = (char (__thiscall*) (void*))0x429F00;

	renameBoxDisplayed = *(bool *)0x8A0CFF;
	if (!renameBoxDisplayed)
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CANCEL);
	else if (update(editingBoxObject) == 1) {
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
		editedDecks[editSelectedDeck].name = (char *)0x8A02F8;
		renameBoxDisplayed = false;
	}
}

void deleteBoxRender()
{
	int text;
	SokuLib::DrawUtils::Sprite textSprite;
	SokuLib::DrawUtils::Sprite &yes = deleteBoxSelectedItem ? yesSelectedSprite : yesSprite;
	SokuLib::DrawUtils::Sprite &no  = deleteBoxSelectedItem ? noSprite : noSelectedSprite;

	if (deleteBoxSelectedItem == 2)
		return;

	baseSprite.setPosition({160, 192});
	baseSprite.setSize(baseSprite.texture.getSize());
	baseSprite.rect = {
		0, 0,
		static_cast<int>(baseSprite.texture.getSize().x),
		static_cast<int>(baseSprite.texture.getSize().y)
	};
	baseSprite.tint = SokuLib::Color::White;
	baseSprite.draw();

	yes.setPosition({242, 228});
	yes.setSize(yes.texture.getSize());
	yes.rect = {
		0, 0,
		static_cast<int>(yes.texture.getSize().x),
		static_cast<int>(yes.texture.getSize().y)
	};
	yes.tint = SokuLib::Color::White;
	yes.draw();

	no.setPosition({338, 228});
	no.setSize(no.texture.getSize());
	no.rect = {
		0, 0,
		static_cast<int>(no.texture.getSize().x),
		static_cast<int>(no.texture.getSize().y)
	};
	no.tint = SokuLib::Color::White;
	no.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, ("Delete deck " + editedDecks[editSelectedDeck].name + " ?").c_str(), defaultFont, TEXTURE_SIZE, FONT_HEIGHT + 18, nullptr, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	textSprite.setPosition({164, 202});
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = SokuLib::Color::White;
	textSprite.fillColors[2] = textSprite.fillColors[3] = SokuLib::Color::Blue;
	textSprite.draw();
}

void openDeleteBox()
{
	SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
	deleteBoxDisplayed = true;
	deleteBoxSelectedItem = 0;
}

void deleteBoxUpdate(SokuLib::KeyManager *keys)
{
	auto horizontal = abs(keys->keymapManager->input.horizontalAxis);

	SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>()->guideVector[2].active = true;
	if (deleteBoxSelectedItem == 2) {
		if (!keys->keymapManager->input.a)
			deleteBoxDisplayed = false;
		return;
	}
	if (keys->keymapManager->input.b || SokuLib::checkKeyOneshot(1, false, false, false)) {
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CANCEL);
		deleteBoxDisplayed = false;
	}
	if (horizontal == 1 || (horizontal >= 36 && horizontal % 6 == 0)) {
		deleteBoxSelectedItem = !deleteBoxSelectedItem;
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
	}
	if (keys->keymapManager->input.a == 1) {
		if (deleteBoxSelectedItem) {
			editedDecks.erase(editedDecks.begin() + editSelectedDeck);
			loadDeckToGame(SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>(), editedDecks[editSelectedDeck].cards);
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
		} else
			SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CANCEL);
		deleteBoxSelectedItem = 2;
	}
}

void copyBoxRender()
{
	int text;
	int width;
	SokuLib::DrawUtils::Sprite textSprite;

	baseSprite.setPosition({160, 192});
	baseSprite.setSize(baseSprite.texture.getSize());
	baseSprite.rect = {
		0, 0,
		static_cast<int>(baseSprite.texture.getSize().x),
		static_cast<int>(baseSprite.texture.getSize().y)
	};
	baseSprite.tint = SokuLib::Color::White;
	baseSprite.draw();

	const std::string &name = copyBoxSelectedItem == editedDecks.size() - 1 ? "Default deck" : editedDecks[copyBoxSelectedItem].name;

	if (!SokuLib::textureMgr.createTextTexture(&text, name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}

	constexpr float increase = 1;
	SokuLib::Vector2i pos{static_cast<int>(321 - (width / 2) * increase), 230};

	textSprite.setPosition(pos);
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({static_cast<unsigned>(TEXTURE_SIZE * increase), static_cast<unsigned>((FONT_HEIGHT + 18) * increase)});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = SokuLib::Color::White;
	textSprite.draw();

	pos.x -= 32 * increase;
	pos.y -= 6 * increase;
	arrowSprite.rect = {0, 0, 32, 32};
	arrowSprite.setPosition(pos);
	arrowSprite.setSize({static_cast<unsigned>(32 * increase + 1), static_cast<unsigned>(32 * increase + 1)});
	arrowSprite.tint = SokuLib::Color::White;
	arrowSprite.draw();

	pos.x += 32 * increase + width * increase;
	arrowSprite.rect.left = 32;
	arrowSprite.setPosition(pos);
	arrowSprite.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, "Choose a deck to copy", defaultFont, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	textSprite.setPosition({166, 200});
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.fillColors[2] = textSprite.fillColors[3] = SokuLib::Color::Blue;
	textSprite.draw();
}

void openCopyBox(SokuLib::ProfileDeckEdit *This)
{
	SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
	copyBoxDisplayed = true;
	copyBoxSelectedItem = 0;
}

void copyBoxUpdate(SokuLib::KeyManager *keys)
{
	if (keys->keymapManager->input.b || SokuLib::checkKeyOneshot(1, false, false, false)) {
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CANCEL);
		copyBoxDisplayed = false;
	}

	auto horizontal = abs(keys->keymapManager->input.horizontalAxis);

	SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>()->guideVector[2].active = true;
	if (horizontal == 1 || (horizontal >= 36 && horizontal % 6 == 0)) {
		if (keys->keymapManager->input.horizontalAxis < 0) {
			if (copyBoxSelectedItem == 0)
				copyBoxSelectedItem = editedDecks.size() - 1;
			else
				copyBoxSelectedItem--;
		} else {
			if (copyBoxSelectedItem == editedDecks.size() - 1)
				copyBoxSelectedItem = 0;
			else
				copyBoxSelectedItem++;
		}
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_MOVE);
	}
	if (keys->keymapManager->input.a == 1) {
		auto menu = SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>();

		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		loadDeckToGame(SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>(), copyBoxSelectedItem == editedDecks.size() - 1 ? defaultDecks[menu->editedCharacter] : editedDecks[copyBoxSelectedItem].cards);
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		copyBoxDisplayed = false;
		openRenameBox(menu);
	}
}

static void loadDefaultDecks()
{
	char buffer[] = "data/csv/000000000000/deck.csv";

	for (auto [id, name] : names) {
		sprintf(buffer, "data/csv/%s/deck.csv", name.c_str());
		printf("Loading default deck %s\n", buffer);

		SokuLib::CSVParser parser{buffer};
		std::array<unsigned short, 20> deck;
		auto &cards = characterSpellCards[id];

		for (int i = 0; i < 20; i++) {
			auto str = parser.getNextCell();

			try {
				auto card = std::stoul(str);

				if (card > 20 && std::find(cards.begin(), cards.end(), card) == cards.end())
					MessageBoxA(
						SokuLib::window,
						("Warning: Default deck for " + name + " contains invalid card " + str).c_str(),
						"Default deck invalid",
						MB_ICONWARNING
					);
				deck[i] = card;
			} catch (std::exception &e) {
				MessageBoxA(
					SokuLib::window,
					(
						"Fatal error: Cannot load default deck for " + name + ":\n" +
						"In file " + buffer + ": Cannot parse cell #" + std::to_string(i + 1) +
						" \"" + str + "\": " + e.what()
					).c_str(),
					"Loading default deck failed",
					MB_ICONERROR
				);
				abort();
			}
		}
		std::sort(deck.begin(), deck.end());
		defaultDecks[id] = deck;
	}
}

static void loadAllExistingCards()
{
	char buffer[] = "data/csv/000000000000/spellcard.csv";

	for (auto [id, name] : names) {
		sprintf(buffer, "data/csv/%s/spellcard.csv", name.c_str());
		printf("Loading cards from %s\n", buffer);

		SokuLib::CSVParser parser{buffer};
		std::vector<unsigned short> valid;
		int i = 0;

		do {
			auto str = parser.getNextCell();

			i++;
			try {
				valid.push_back(std::stoul(str));
			} catch (std::exception &e) {
				MessageBoxA(
					SokuLib::window,
					(
						"Fatal error: Cannot load cards list for " + name + ":\n" +
						"In file " + buffer + ": Cannot parse cell #1 at line #" + std::to_string(i) +
						" \"" + str + "\": " + e.what()
					).c_str(),
					"Loading default deck failed",
					MB_ICONERROR
				);
				abort();
			}
		} while (parser.goToNextLine());
		characterSpellCards[id] = valid;
	}
}

void loadProfile(const char *path, SokuLib::Profile *profileObj, const char *defaultVal, unsigned char r, unsigned char g, unsigned char b)
{
	auto initProfile = (bool (__thiscall *)(SokuLib::Profile *))0x4358C0;
	auto saveProfile = (bool (__thiscall *)(SokuLib::Profile *, const char *path))0x434FB0;
	auto changeProfile = (bool (__thiscall *)(SokuLib::Profile *, const char *path))0x435300;
	std::ifstream stream{path};
	std::string profile;
	auto createProfileSprite = (void (__thiscall *)(SokuLib::Profile *This, unsigned char r, unsigned char g, unsigned char b))0x434C80;

	stream >> profile;
	if (!stream)
		profile = defaultVal;
	if (!changeProfile(profileObj, (profile + ".pf").c_str())) {
		if (defaultVal == profile || changeProfile(profileObj, (std::string(defaultVal) + ".pf").c_str())) {
			initProfile(profileObj);
			saveProfile(profileObj, (std::string(defaultVal) + ".pf").c_str());
		}
	}
	createProfileSprite(profileObj, r, g, b);
}

void reloadProfile(SokuLib::Profile *profileObj, const char *defaultVal, unsigned char r, unsigned char g, unsigned char b)
{
	auto initProfile = (void (__thiscall *)(SokuLib::Profile *))0x4358C0;
	auto reloadProfile = (bool (__thiscall *)(SokuLib::Profile *))0x4355F0;
	auto saveProfile = (void (__thiscall *)(SokuLib::Profile *, const char *path))0x434FB0;
	auto changeProfile = (bool (__thiscall *)(SokuLib::Profile *, const char *path))0x435300;
	auto createProfileSprite = (void (__thiscall *)(SokuLib::Profile *This, unsigned char r, unsigned char g, unsigned char b))0x434C80;

	if (!reloadProfile(profileObj)) {
		if (changeProfile(profileObj, defaultVal)) {
			initProfile(profileObj);
			saveProfile(profileObj, defaultVal);
		}
	}
	createProfileSprite(profileObj, r, g, b);
}

static void saveProfiles()
{
	std::ofstream s1{"profile3p.txt"};
	std::ofstream s2{"profile4p.txt"};

	s1 << loadedProfiles[2];
	s2 << loadedProfiles[3];
}

static void reloadProfiles()
{
	reloadProfile(profiles[2], "profile3p.pf", 0xA0, 0xA0, 0xFF);
	reloadProfile(profiles[3], "profile4p.pf", 0xFF, 0x80, 0x80);
}

static void loadExtraAssets()
{
	SokuLib::Vector2i size;
	const char *cursorSprites[] = {
		"data/scene/select/character/04c_cursor1p.bmp",
		"data/scene/select/character/04c_cursor2p.bmp",
		"data/scene/select/character/04c_cursor3p.bmp",
		"data/scene/select/character/04c_cursor4p.bmp"
	};
	const char *cursorSprites2[] = {
		"data/scene/select/character/04b_cursor1p.bmp",
		"data/scene/select/character/04b_cursor2p.bmp",
		"data/scene/select/character/04b_cursor3p.bmp",
		"data/scene/select/character/04b_cursor4p.bmp"
	};
	const char *healingSprites[] = {
		"data/character/common/breakEF_in002.bmp",
		"data/character/common/breakEF_out002.bmp"
	};
	SokuLib::SWRFont font;
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = FONT_HEIGHT;
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 2;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, SokuLib::defaultFontName);
	font.create();
	font.setIndirect(desc);

	for (int i = 0; i < 4; i++) {
		cursors[i].texture.loadFromGame(cursorSprites[i]);
		cursors[i].setSize(cursors[i].texture.getSize());
		cursors[i].rect.width = cursors[i].getSize().x;
		cursors[i].rect.height = cursors[i].getSize().y;
	}
	for (int i = 0; i < 4; i++) {
		cursors2[i].texture.loadFromGame(cursorSprites2[i]);
		cursors2[i].setSize(cursors2[i].texture.getSize());
		cursors2[i].rect.width = cursors2[i].texture.getSize().x;
		cursors2[i].rect.height = cursors2[i].texture.getSize().y;
	}
	for (int i = 0; i < 2; i++) {
		healingCircle[i].texture.loadFromGame(healingSprites[i]);
		healingCircle[i].setSize({HEAL_RADIUS * 2, HEAL_RADIUS * 2});
		healingCircle[i].rect.width = healingCircle[i].texture.getSize().x;
		healingCircle[i].rect.height = healingCircle[i].texture.getSize().y;
		healingCircle[i].setCamera(&SokuLib::camera);
	}

	inputsSheet.texture.loadFromGame("data/number/inputs.bmp");
	inputsSheet.setSize({24, 24});
	inputsSheet.rect.width = 24;
	inputsSheet.rect.height = 24;

	numbersSheet.texture.loadFromGame("data/number/0000.bmp");
	numbersSheet.setSize({14, 19});
	numbersSheet.rect.width = 14;
	numbersSheet.rect.height = 19;

	chooseBoxBack.texture.loadFromGame("data/menu/21_base.bmp");
	chooseBoxBack.setSize(chooseBoxBack.texture.getSize());
	chooseBoxBack.rect.width = chooseBoxBack.getSize().x;
	chooseBoxBack.rect.height = chooseBoxBack.getSize().y;
	chooseBoxBack.setPosition({
		static_cast<int>(320 - (chooseBoxBack.getSize().x - 12) / 2),
		static_cast<int>(240 - (chooseBoxBack.getSize().y - 12) / 2),
	});

	chooseBoxText.texture.createFromText("Select player slot: ", font, chooseBoxBack.getSize(), &size);
	chooseBoxText.setSize(size.to<unsigned>());
	chooseBoxText.rect.width = size.x;
	chooseBoxText.rect.height = size.y;
	chooseBoxText.setPosition({
		chooseBoxBack.getPosition().x + 10,
		240 - size.y / 2
	});

	chooseBoxWaitText.texture.createFromText("Waiting for players...   ", font, chooseBoxBack.getSize(), &size);
	chooseBoxWaitText.setSize(size.to<unsigned>());
	chooseBoxWaitText.rect.width = size.x;
	chooseBoxWaitText.rect.height = size.y;
	chooseBoxWaitText.setPosition({
		chooseBoxBack.getPosition().x + 10,
		240 - size.y / 2
	});
	font.destruct();
}

static void initAssets()
{
	if (assetsLoaded)
		return;
	assetsLoaded = true;
	loadAllExistingCards();
	loadDefaultDecks();
	loadCardAssets();
	loadExtraAssets();
	loadProfile("profile3p.txt", profiles[2], "profile3p", 0xA0, 0xA0, 0xFF);
	loadProfile("profile4p.txt", profiles[3], "profile4p", 0xFF, 0x80, 0x80);
}

int __fastcall CProfileDeckEdit_OnProcess(SokuLib::ProfileDeckEdit *This)
{
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);
	bool ogDialogsActive = This->guideVector[3].active || This->guideVector[2].active;

	profileSelectReady = false;
	selectDeckGuide.active = This->cursorOnDeckChangeBox && editSelectedDeck != editedDecks.size() - 1 && !ogDialogsActive;
	createDeckGuide.active = This->cursorOnDeckChangeBox && editSelectedDeck == editedDecks.size() - 1 && !ogDialogsActive;
	This->guideVector[4].active = false;
	if (renameBoxDisplayed && ogDialogsActive) {
		renameBoxDisplayed = false;
		((int(__thiscall*) (void*, bool))0x40ea10)((void*)0x8A02F0, true);
	}
	deleteBoxDisplayed &= !ogDialogsActive;
	copyBoxDisplayed   &= !ogDialogsActive;

	bool renameBox = renameBoxDisplayed;
	bool deleteBox = deleteBoxDisplayed;
	bool copyBox   = copyBoxDisplayed;

	editBoxGuide.active = renameBoxDisplayed;
	if (ogDialogsActive);
	else if (renameBox)
		renameBoxUpdate(keys);
	else if (deleteBox)
		deleteBoxUpdate(keys);
	else if (copyBox)
		copyBoxUpdate(keys);
	else if (keys->keymapManager->input.a == 1 && This->cursorOnDeckChangeBox) {
		if (editSelectedDeck == editedDecks.size() - 1)
			openCopyBox(This);
		else
			openRenameBox(This);
	} else if (keys->keymapManager->input.c && This->cursorOnDeckChangeBox && editSelectedDeck != editedDecks.size() - 1)
		openDeleteBox();
	else if (keys->keymapManager->input.c && This->cursorOnDeckChangeBox) {
		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		openRenameBox(This);
	}
	if (renameBox || deleteBox || copyBox) {
		selectDeckGuide.active = false;
		createDeckGuide.active = false;
		This->guideVector[0].active = false;
		escPressed = ((int *)0x8998D8)[1];
		((int *)0x8998D8)[1] = 0;
		((int *)0x8998D8)[DIK_F1] = 0;
		memset(&keys->keymapManager->input, 0, sizeof(keys->keymapManager->input));
	} else if (escPressed) {
		escPressed = ((int *)0x8998D8)[1];
		((int *)0x8998D8)[1] = 0;
	} else if (editSelectedDeck == editedDecks.size() - 1 && This->displayedNumberOfCards != 0) {
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}
	// This hides the deck select arrow
	((bool ***)This)[0x10][0x2][20] = false;
	updateGuide(selectDeckGuide);
	updateGuide(createDeckGuide);
	updateGuide(editBoxGuide);
	return (This->*s_originalCProfileDeckEdit_OnProcess)();
}

int __fastcall CProfileDeckEdit_OnRender(SokuLib::ProfileDeckEdit *This)
{
	int ret = (This->*s_originalCProfileDeckEdit_OnRender)();

	SokuLib::DrawUtils::Sprite &sprite = cardsTextures[SokuLib::CHARACTER_RANDOM][21];
	SokuLib::DrawUtils::Sprite textSprite;
	SokuLib::Vector2i pos{38, 88};
	int text;
	int width = 0;

	if (saveError) {
		saveError = false;
		This->editedDeck->begin()->second--;
	}

	sprite.rect.top = sprite.rect.width = 0;
	sprite.rect.width = sprite.texture.getSize().x;
	sprite.rect.height = sprite.texture.getSize().y;
	sprite.tint = SokuLib::Color::White;
	sprite.setSize({20, 32});
	for (int i = 20; i > This->displayedNumberOfCards; i--) {
		sprite.setPosition({304 + 24 * ((i - 1) % 10), 260 + 38 * ((i - 1) / 10)});
		sprite.draw();
	}

	if (errorCounter) {
		float alpha = min(1.f, errorCounter / 30.f);

		errorCounter--;
		if (!SokuLib::textureMgr.createTextTexture(&text, "Please keep the number of cards in the deck at or below 20", font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
			puts("C'est vraiment pas de chance");
			return ret;
		}

		auto realX = 53;

		if (errorCounter >= 105) {
			realX += sokuRand(31) - 15;
			if (errorCounter >= 115)
				alpha = 1 - (errorCounter - 115.f) / 5;
		}

		textSprite.setPosition({realX, pos.y - 20});
		textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
		textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
		textSprite.rect = {
			0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
		};
		textSprite.tint = SokuLib::Color::Red * alpha;
		textSprite.draw();
	}

	if (!SokuLib::textureMgr.createTextTexture(&text, editedDecks[editSelectedDeck].name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return ret;
	}
	pos.x = 153 - width / 2;
	textSprite.setPosition(pos);
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = SokuLib::Color::White;
	textSprite.draw();

	pos.x -= 32;
	pos.y -= 6;
	arrowSprite.rect = {0, 0, 32, 32};
	arrowSprite.setPosition(pos);
	arrowSprite.setSize({33, 33});
	arrowSprite.tint = SokuLib::Color::White;
	arrowSprite.draw();

	pos.x += 32 + width;
	arrowSprite.rect.left = 32;
	arrowSprite.setPosition(pos);
	arrowSprite.draw();

	renderGuide(selectDeckGuide);
	renderGuide(createDeckGuide);
	renderGuide(editBoxGuide);
	if (renameBoxDisplayed)
		renameBoxRender();
	else if (deleteBoxDisplayed)
		deleteBoxRender();
	else if (copyBoxDisplayed)
		copyBoxRender();
	return ret;
}

SokuLib::ProfileDeckEdit *__fastcall CProfileDeckEdit_Destructor(SokuLib::ProfileDeckEdit *This, int, unsigned char param)
{
	auto setup_global = (int(__thiscall*) (void*, bool))0x40ea10;

	setup_global((void*)0x8A02F0, false);
	return (This->*s_originalCProfileDeckEdit_Destructor)(param);
}

SokuLib::ProfileDeckEdit *__fastcall CProfileDeckEdit_Init(SokuLib::ProfileDeckEdit *This, int, int param_2, int param_3, SokuLib::Sprite *param_4)
{
	auto ret = (This->*og_CProfileDeckEdit_Init)(param_2, param_3, param_4);

	if (profileSelectReady)
		return ret;
	profileSelectReady = true;
	errorCounter = 0;
	editSelectedDeck = 0;
	if (editSelectedProfile != 4) {
		loadedDecks[4] = loadedDecks[editSelectedProfile];
		for (auto &val : loadedDecks[4])
			val.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	} else if (loadedDecks[4][This->editedCharacter].empty()) {
		loadedDecks[4] = loadedDecks[0];
		for (auto &val : loadedDecks[4])
			val.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}
	editedDecks = loadedDecks[4][This->editedCharacter];
	loadDeckToGame(This, editedDecks[editSelectedDeck].cards);
	deleteBoxDisplayed = false;
	renameBoxDisplayed = false;
	copyBoxDisplayed = false;
	return ret;
}

static void sanitizeDeck(SokuLib::Character chr, Deck &deck)
{
	unsigned last = 100 + nbSkills[chr];
	std::map<unsigned short, unsigned char> used;
	std::vector<unsigned short> cards;
	auto name = names.find(chr);

	if (name == names.end())
		return;
	for (int i = 0; i <= 21; i++)
		cards.push_back(i);
	for (int i = 100; i < last; i++)
		cards.push_back(i);
	for (auto &card : characterSpellCards[chr])
		cards.push_back(card);
	for (int i = 0; i < 20; i++) {
		auto &card = deck.cards[i];

		if (card == 21)
			continue;
		if (std::find(cards.begin(), cards.end(), card) == cards.end() || used[card] >= 4) {
			card = 21;
			continue;
		}
		used[card]++;
	}
}

static bool loadOldProfileFile(nlohmann::json &json, std::map<unsigned char, std::vector<Deck>> &map, int index)
{
	if (json.size() != 20)
		throw std::invalid_argument("Not 20 characters");
	for (auto &arr : json) {
		for (auto &elem : arr) {
			elem.contains("name") && (elem["name"].get<std::string>(), true);
			if (!elem.contains("cards") || elem["cards"].get<std::vector<unsigned short>>().size() != 20)
				throw std::invalid_argument(elem.dump());
		}
	}
	for (auto &elem : map)
		elem.second.clear();
	for (int i = 0; i < 20; i++) {
		auto &array = json[i];

		for (int j = 0; j < array.size(); j++) {
			auto &elem = array[j];
			Deck deck;

			if (!elem.contains("name"))
				deck.name = "Deck #" + std::to_string(j + 1);
			else
				deck.name = elem["name"];
			memcpy(deck.cards.data(), elem["cards"].get<std::vector<unsigned short>>().data(), sizeof(*deck.cards.data()) * deck.cards.size());
			sanitizeDeck(static_cast<SokuLib::Character>(i), deck);
			std::sort(deck.cards.begin(), deck.cards.end());
			map[i].push_back(deck);
		}
		if (index == 4)
			map[i].push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}
	return true;
}

bool allDecksDefault(unsigned short (*decks)[4][20], unsigned i)
{
	for (int j = 0; j < 4; j++) {
		std::sort(decks[i][j], decks[i][j] + 20);
		if (i >= (hasSoku2 ? defaultDecks.size() + 2 : 20) || memcmp(defaultDecks[i].data(), decks[i][j], sizeof(defaultDecks[i])) != 0)
			return false;
	}
	return true;
}

static bool convertProfile(const char *jsonPath)
{
	char path[MAX_PATH];
	unsigned char length;
	unsigned short cards[255];
	FILE *json;
	FILE *profile;
	unsigned short (*decks)[4][20] = nullptr;
	int size;

	strcpy(path, jsonPath);
	*strrchr(path, '.') = 0;
	strcat(path, ".pf");
	printf("Loading decks from profile file %s to %s.\n", path, jsonPath);

	profile = fopen(path, "r");
	if (!profile) {
		printf("Can't open %s for reading %s\n", path, strerror(errno));
		return false;
	}

	json = fopen(jsonPath, "w");
	if (!json) {
		fclose(profile);
		printf("Can't open %s for writing %s\n", jsonPath, strerror(errno));
		return false;
	}

	fseek(profile, 106, SEEK_SET);
	for (size = 1; !feof(profile); size++) {
		decks = static_cast<unsigned short (*)[4][20]>(realloc(decks, sizeof(*decks) * size));
		for (int k = 0; k < 4; k++) {
			fread(&length, sizeof(length), 1, profile);
			fread(cards, sizeof(*cards), length, profile);
			for (int j = length; j < 20; j++)
				cards[j] = 21;
			memcpy(decks[size - 1][k], cards, 40);
		}
	}
	fclose(profile);

	fwrite("{", 1, 1, json);

	const char *deckNames[4] = {
		"yorokobi",
		"ikari",
		"ai",
		"tanoshii"
	};
	bool first = true;
	bool first2 = true;
	unsigned i = 0;

	size -= 2;
	if (size > 20)
		size -= 2;
	printf("There are %i characters...\n", size);
	while (size--) {
		if (allDecksDefault(decks, i)) {
			printf("Character %i has all default decks\n", i);
			i++;
			if (i == 20)
				i += 2;
			continue;
		}
		fprintf(json, "%s\n\t\"%i\": [", first2 ? "" : ",", i);
		first2 = false;
		first = true;
		for (int j = 0; j < 4; j++) {
			std::sort(decks[i][j], decks[i][j] + 20);
			if (i < (hasSoku2 ? defaultDecks.size() + 2 : 20) && memcmp(defaultDecks[i].data(), decks[i][j], sizeof(defaultDecks[i])) == 0)
				continue;
			fprintf(json, "%s\n\t\t{\n\t\t\t\"name\": \"%s\",\n\t\t\t\"cards\": [", first ? "" : ",", deckNames[j]);
			first = false;
			for (int k = 0; k < 20; k++)
				fprintf(json, "%s%i", k == 0 ? "" : ", ", decks[i][j][k]);
			fwrite("]\n\t\t}", 1, 5, json);
		}
		fwrite("\n\t]", 1, 3, json);
		i++;
		if (i == 20)
			i += 2;
	}
	fwrite("\n}", 1, 2, json);
	fclose(json);
	free(decks);
	decks = nullptr;
	return true;
}

static bool saveProfile(const std::string &path, const std::map<unsigned char, std::vector<Deck>> &profile)
{
	nlohmann::json result;

	printf("Saving to %s\n", path.c_str());
	for (auto &elem : profile) {
		if (elem.second.empty())
			continue;

		auto &array = result[std::to_string(elem.first)];

		array = nlohmann::json::array();
		for (auto &deck : elem.second) {
			array.push_back({
				{"name", deck.name},
				{"cards", std::vector<unsigned short>{
					deck.cards.begin(),
					deck.cards.end()
				}}
			});
		}
	}
	if (std::ifstream(path + ".bck").fail())
		rename(path.c_str(), (path + ".bck").c_str());

	auto resultStr = result.dump(4);
	std::ofstream stream{path};

	if (stream.fail()) {
		MessageBoxA(SokuLib::window, ("Cannot open \"" + path + "\". Please make sure you have proper permissions and enough space on disk.").c_str(), "Saving error", MB_ICONERROR);
		return false;
	}
	stream << resultStr;
	if (stream.fail()) {
		stream.close();
		MessageBoxA(SokuLib::window, ("Cannot write to \"" + path + "\". Please make sure you have proper enough space on disk.").c_str(), "Saving error", MB_ICONERROR);
		return false;
	}
	stream.close();
	unlink((path + ".bck").c_str());
	return true;
}

static bool loadProfileFile(const std::string &path, std::ifstream &stream, std::map<unsigned char, std::vector<Deck>> &map, int index, bool hasBackup = false)
{
	if (stream.fail()) {
		printf("Failed to open file %s: %s\n", path.c_str(), strerror(errno));
		if (hasBackup)
			throw std::exception();
		if (errno == ENOENT) {
			puts("Let's fix that");
			if (convertProfile(path.c_str())) {
				stream.open(path);
				if (stream)
					return loadProfileFile(path, stream, map, index);
			}
		}
		for (auto &elem : names)
			map[elem.first].clear();
		if (index == 4)
			for (auto &elem : map)
				elem.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		return false;
	}

	nlohmann::json json;

	stream >> json;
	if (json.is_array()) {
		printf("%s is in the old format. Converting...\n", path.c_str());
		loadOldProfileFile(json, map, index);
		stream.close();
		saveProfile(path, map);
		return true;
	}
	if (json.is_null()) {
		for (auto &elem : names)
			map[elem.first].clear();
		if (index == 4)
			for (auto &elem : map)
				elem.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		return true;
	}
	if (!json.is_object())
		throw std::invalid_argument("JSON is neither an array nor an object");
	for (auto &arr : json.items()) {
		(void)std::stoi(arr.key());
		for (auto &elem : arr.value()) {
			elem.contains("name") && (elem["name"].get<std::string>(), true);
			if (!elem.contains("cards") || elem["cards"].get<std::vector<unsigned short>>().size() != 20)
				throw std::invalid_argument(elem.dump());
		}
	}
	for (auto &elem : names)
		map[elem.first].clear();
	for (auto &arr : json.items()) {
		auto &array = arr.value();
		auto chr = std::stoi(arr.key());

		for (int j = 0; j < array.size(); j++) {
			auto &elem = array[j];
			Deck deck;

			if (!elem.contains("name"))
				deck.name = "Deck #" + std::to_string(j + 1);
			else
				deck.name = elem["name"];
			memcpy(deck.cards.data(), elem["cards"].get<std::vector<unsigned short>>().data(), sizeof(*deck.cards.data()) * deck.cards.size());
			sanitizeDeck(static_cast<SokuLib::Character>(chr), deck);
			std::sort(deck.cards.begin(), deck.cards.end());
			map[chr].push_back(deck);
		}
	}
	if (index == 4)
		for (auto &elem : map)
			elem.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	return true;
}

static void __fastcall handleProfileChange(SokuLib::Profile *This, SokuLib::String *val)
{
	initAssets();

	const char *arg = *val;
	std::string profileName{arg, strstr(arg, ".pf")};
	std::string profile = "profile/" + profileName + ".json";
	int index = 0;
	bool hasBackup;

	while (index < 4 && This != profiles[index])
		index++;
	if (index != 4)
		loadedProfiles[index] = profileName;
	printf("Loading %s in buffer %i\n", profile.c_str(), index);

	bool result = false;
	auto &arr = loadedDecks[index];
	std::ifstream stream{profile + ".bck"};

	hasBackup = !stream.fail();
	stream.close();
	if (hasBackup)
		printf("%s has backup data !\n", profile.c_str());

	stream.open(profile, std::ifstream::in);
	try {
		result = loadProfileFile(profile, stream, arr, index, hasBackup);
	} catch (std::exception &e) {
		auto answer = IDNO;

		if (!hasBackup)
			MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ": " + e.what()).c_str(), "Fatal error", MB_ICONERROR);
		else
			answer = MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ": " + e.what() + "\n\nDo you want to load backup file ?").c_str(), "Loading error", MB_ICONERROR | MB_YESNO);
		if (answer != IDYES) {
			try {
				result = loadProfileFile(profile, stream, arr, index);
			} catch (std::exception &e) {
				MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ": " + e.what()).c_str(), "Fatal error", MB_ICONERROR);
				throw;
			}
		}
	}
	stream.close();

	if (!result && hasBackup) {
		try {
			stream.open(profile + ".bck", std::ifstream::in);
			printf("Loading %s\n", (profile + ".bck").c_str());
			result = loadProfileFile(profile + ".bck", stream, arr, index);
			stream.close();
		} catch (std::exception &e) {
			MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ".bck: " + e.what()).c_str(), "Fatal error", MB_ICONERROR);
			throw;
		}
		unlink(profile.c_str());
		rename((profile + ".bck").c_str(), profile.c_str());
		lastLoadedProfile = profileName;
		return;
	}

	lastLoadedProfile = profileName;
	if (hasBackup)
		unlink((profile + ".bck").c_str());
}

void __declspec(naked) onProfileChanged()
{
	__asm {
		MOV ECX, ESI;
		MOV EDX, [ESP + 0x18];
		JMP handleProfileChange
	}
}

static void onDeckSaved()
{
	auto menu = SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>();
	std::string path;

	if (!saveDeckFromGame(menu, editedDecks[editSelectedDeck].cards))
		return;

	loadedDecks[4][menu->editedCharacter] = editedDecks;

	auto toSave = loadedDecks[4];

	for (auto &elem : toSave)
		elem.second.pop_back();
	if (editSelectedProfile != 4) {
		loadedDecks[editSelectedProfile] = toSave;
		path = "profile/" + loadedProfiles[editSelectedProfile] + ".json";
	} else
		path = "profile/" + lastLoadedProfile + ".json";

	if (!saveProfile(path, toSave)) {
		if (menu->displayedNumberOfCards == 20) {
			menu->editedDeck->begin()->second++;
			saveError = true;
		}
		return;
	}

	for (auto &card : *menu->editedDeck)
		card.second = 0;
	for (int i = 0; i < 20; i++)
		(*menu->editedDeck)[i] = 1;
}

void __declspec(naked) getProfileInfo()
{
	__asm {
		MOV EAX, [profiles + EAX * 4]
		RET
	}
}

void drawGradiantBar(float x, float y, float maxY)
{
	if (y == 114)
		y = 90;
	s_originalDrawGradiantBar(x, y, maxY);
}

void onMenuTopProfileInit()
{
	((SokuLib::CDesign *)0x89A68C)->getById(&profileHandlers[0], 102);
	((SokuLib::CDesign *)0x89A68C)->getById(&profileHandlers[1], 103);
	if (profileHandlers[0] && profileHandlers[1]) {
		profileHandlers[0]->active = true;
		profileHandlers[1]->active = true;
	} else
		MessageBoxA(SokuLib::window, "The assets for 4PSoku are not found or corrupted. The game will launch in an unstable state.", "4PSoku error", MB_ICONERROR);
}

void renderExtraProfilesTop()
{
	for (int i = 0; i < 2; i++) {
		SokuLib::Sprite &sprite = profiles[i + 2]->sprite;

		sprite.setColor((*(unsigned char *)0x89A457 << 24) | 0xFFFFFF);
		sprite.render(profileHandlers[i]->x2 + 88, profileHandlers[i]->y2 + 10);
		sprite.setColor(0xFFFFFFFF);
	}
}

void __declspec(naked) selectProfileSpriteColor()
{
	__asm {
		MOVSX EAX, byte ptr [EDI + 0x70]
		MOV ECX, EAX
		AND ECX, 1
		RET
	}
}

void chrSelect_pushActiveInputs()
{
	SokuLib::Select *This;
	auto unkSTL_push = (void (__thiscall *)(void *, SokuLib::KeymapManager **))0x44BFE0;

	__asm MOV This, ESI;
	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER) {
		chrSelectExtra[0].input = &extraOnlineInputs[0];
		chrSelectExtra[1].input = &extraOnlineInputs[1];
	}
	if (chrSelectExtra[0].input)
		unkSTL_push(&This->offset_0x018[0x60], &chrSelectExtra[0].input);
	if (chrSelectExtra[1].input)
		unkSTL_push(&This->offset_0x018[0x60], &chrSelectExtra[1].input);
}

void __fastcall turnAroundHitBlock(SokuLib::CharacterManager *This, SokuLib::ObjectManager *op)
{
	float fVar1;
	float fVar2;
	char cVar3;
	SokuLib::CharacterManager *pPVar4;

	cVar3 = This->objectBase.direction;
	pPVar4 = op->owner;
	if (This->objectBase.position.x < pPVar4->objectBase.position.x)
		This->objectBase.direction = SokuLib::RIGHT;
	else if (This->objectBase.position.x > pPVar4->objectBase.position.x)
		This->objectBase.direction = SokuLib::LEFT;
}

void __declspec(naked) turnAroundHitBlock_hook()
{
	__asm {
		MOV EDX, EDI
		JMP turnAroundHitBlock
	}
}

void __declspec(naked) getCharacterIndexResetHealth_hook() {
	__asm {
		MOV ESI, 0x8985E4 //CBattleManagerPtr
		MOV ESI, [ESI]
		MOV ESI, dword ptr [ESI + EDI * 0x4 + 0xC]
		MOV ECX, ESI
		RET
	}
}

void __declspec(naked) setGrazeFlags()
{
	__asm {
		CMP dword ptr [EDI + 0x190], 0
		JNZ _ret
		MOV dword ptr [EDI + 0x190], 0x6
	_ret:
		RET
	}
}

void __declspec(naked) swapComboCtrs()
{
	__asm {
		POP        EDX

		MOV        EAX, dword ptr [ESP + 0x14]
		LEA        EAX, [EAX + ESI * 0x4]
		MOV        ECX, 0x8985E4
		ADD        EAX, dword ptr [ECX]
		MOV        EAX, dword ptr [ESP + EAX * 0x1 + 0x1c]
		MOV        dword ptr [ESP + ESI * 0x4 + 0x1c], EAX

		MOV        EAX, dword ptr [ESP + 0x14]
		PUSH       ESI
		XOR        ESI, 1
		LEA        EAX, [EAX + ESI * 0x4]
		MOV        ESI, 0x8985E4
		ADD        EAX, dword ptr [ESI]
		POP        ESI
		MOV        EAX, dword ptr [ESP + EAX * 0x1 + 0x1c]

		MOV        ECX, dword ptr [ESP + 0x18]
		PUSH       EAX
		PUSH       ECX
		MOV        ECX, EDI
		JMP        EDX
	}
}

void resetOnlineCharacterSelect()
{
	extraOnlineInputsBuffer[0].clear();
	extraOnlineInputsBuffer[1].clear();
	characterSelectLocked = true;
	mySlot = -1;
	remoteMySlot = -1;
	selectedSlot = 0;
	slotsTaken[0] = false;
	slotsTaken[1] = false;
	slotsTaken[2] = false;
	slotsTaken[3] = false;
}

void __declspec(naked) copyExtraProfileNames()
{
	__asm {
		LEA EDI, [extraOnlineNames]
		MOV ECX, 0x10
		REP MOVSD
		PUSH EAX
		CALL resetOnlineCharacterSelect
		POP EAX
		MOV EDX, dword ptr [EAX + 0xFFFFF960]
		RET
	}
}

void reloadNetProfileTexture(int index)
{
	SokuLib::SWRFont font;
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.weight = FW_NORMAL;
	desc.italic = 0;
	desc.shadow = 1;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 2;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;

	strcpy(desc.faceName, *(char **)0x453BFA);
	if (index % 2 == 0) {
		desc.r2 = 0xA0;
		desc.g2 = 0xA0;
	} else {
		desc.g2 = 0x80;
		desc.b2 = 0x80;
	}

	int r = 0;
	int old = 0;
	int *final;
	const char *name;
	SokuLib::Sprite *sprite;
	size_t size;

	if (index < 2) {
		auto &netObject = SokuLib::getNetObject();

		name = netObject.profile1name + (index * 32);
		sprite = &netObject.p1ProfileNameSprite + index;
		final = &netObject.p1ProfileNameTexture + index;
	} else {
		name = extraOnlineNames[index - 2];
		sprite = &extraNetProfileNames[index - 2];
		final = &extraNetProfileNamesTextures[index - 2];
	}
	size = strlen(name);
	if (memcmp(emptyMagicString, name, 32) == 0) {
		name = emptyName;
		size = strlen("[Empty Slot]");
		slotsTaken[index] = false;
	} else
		slotsTaken[index] = true;

	int v = ((int)size - 16) / 2;

	v = v & (v < 0) - 1;
	desc.height = 14 - v;
	desc.offsetY = v / 2;

	font.create();
	font.setIndirect(desc);
	old = *final;
	SokuLib::textureMgr.createTextTexture(&r, name, font, 0x80, 0x20, nullptr, nullptr);
	sprite->setTexture2(r, 0, 0, 0x80, 0x20);
	*final = r;
	if (old)
		SokuLib::textureMgr.remove(old);
	font.destruct();
}

void __declspec(naked) inputMul4()
{
	__asm {
		ADD ESI, ESI
		ADD ESI, ESI
		CMP ESI, dword ptr [EDI + 0x20]
		RET
	}
}

void pushOnlineInputs(SokuLib::Deque<unsigned short> &base, SokuLib::NetObject &obj, int index)
{
	obj.p1Inputs.push_back(base.at(index + 0));
	obj.p2Inputs.push_back(base.at(index + 1));
	extraOnlineInputsBuffer[0].push_back(base.at(index + 2));
	extraOnlineInputsBuffer[1].push_back(base.at(index + 3));
}

void __fastcall updateExtraInputHandlers(void *This)
{
	const auto fct = (void (__thiscall *)(void *))0x4549F0;
	auto &netObj = SokuLib::getNetObject();

	fct(This);
	// front() on an empty deque is unchecked here -- it dereferences begin()
	// and hands back whatever is at that address.
	//
	// The pushes come from pushOnlineInputs, four per received frame, and they
	// are not guaranteed to keep up with the pops any more. The relay reflects
	// each sender's own datagram now instead of emitting one 4-wide array, and
	// giuroll's canned startup packets declare fewer inputs than one 4P frame
	// needs, which yields zero pushes while this still pops once per frame.
	//
	// Hold the previous input when the queue is dry: a repeated frame is a
	// visible stutter that recovers on the next packet, a garbage read is not.
	for (int i = 0; i < 2; i++) {
		if (extraOnlineInputsBuffer[i].empty())
			continue;
		extraOnlineInputs[i].inKeys.raw = extraOnlineInputsBuffer[i].front();
		extraOnlineInputsBuffer[i].pop_front();
	}
	replayInputs.push_back({
		netObj.p1InputMgr.inKeys,
		netObj.p2InputMgr.inKeys,
		extraOnlineInputs[0].inKeys,
		extraOnlineInputs[1].inKeys
	});
}

void __fastcall onInputsReset(void *This)
{
	og_FUNCALL43F996(This);
	extraOnlineInputsBuffer[0].clear();
	extraOnlineInputsBuffer[1].clear();
	replayInputs.clear();
}

constexpr auto __keyManagerSize = sizeof(*extraOnlineInputs);

void __declspec(naked) updateExtraOnlineInputs()
{
	__asm {
		MOV EDX, dword ptr [EAX + 0x4]
		CALL EDX
		LEA ECX, dword ptr [extraOnlineInputs]
		MOV EAX, dword ptr [ECX]
		MOV EDX, dword ptr [EAX + 0x4]
		CALL EDX
		MOV ECX, [__keyManagerSize]
		LEA ECX, dword ptr [extraOnlineInputs + ECX]
		MOV EAX, dword ptr [ECX]
		MOV EDX, dword ptr [EAX + 0x4]
		JMP EDX
	}
}

void __fastcall handlePlayerJoinPacket(PacketPlayerJoin &packet, size_t packetSize)
{
	if (packetSize < sizeof(packet))
		return;
	if (packet.slot == -1) {
		SokuLib::playSEWaveBuffer(SokuLib::SFX_NETBELL);
		printf("%s joined.\n", packet.name);
		return;
	}
	if (packet.slot < -1 || packet.slot > 3)
		return;

	char *name;

	if (packet.slot < 2)
		name = SokuLib::getNetObject().profile1name + (packet.slot * 32);
	else
		name = extraOnlineNames[packet.slot - 2];
	if (packet.slot == mySlot && mySlot != remoteMySlot)
		mySlot = -2;
	if (memcmp(packet.name, emptyMagicString, 32) == 0) {
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CANCEL);
		characterSelectLocked = true;
	} else
		SokuLib::playSEWaveBuffer(SokuLib::SFX_MENU_CONFIRM);
	memcpy(name, packet.name, sizeof(packet.name));
	reloadNetProfileTexture(packet.slot);
}

void __fastcall handlePlayerJoinAckPacket(PacketPlayerJoinAck &packet, size_t packetSize)
{
	if (packetSize < sizeof(packet))
		return;
	if (packet.slot < -1 || packet.slot > 3)
		return;
	if (packet.slot >= 0)
		slotsTaken[packet.slot] = true;
	mySlot = packet.slot;
	remoteMySlot = packet.slot;
}

void handleUnlockCharSelectPacket()
{
	characterSelectLocked = false;
}

void __declspec(naked) handlePlayerJoinPacket_hook()
{
	__asm {
		LEA ECX, [ESP + 0x70]
		MOV EDX, [ESP + 0x14]
		PUSH 0x41DFC2
		JMP handlePlayerJoinPacket
	}
}

void __declspec(naked) handlePlayerJoinAckPacket_hook()
{
	__asm {
		LEA ECX, [ESP + 0x70]
		MOV EDX, [ESP + 0x14]
		PUSH 0x41DFC2
		JMP handlePlayerJoinAckPacket
	}
}

void __declspec(naked) handleUnlockCharSelectPacket_hook()
{
	__asm {
		PUSH 0x41DFC2
		JMP handleUnlockCharSelectPacket
	}
}

void sendCurrentSelectCharacter()
{
	auto &netobj = SokuLib::getNetObject();
	SokuLib::PlayerInfo *info = nullptr;

	generateFakeDecks();
	switch (mySlot) {
	case 0:
		info = &SokuLib::leftPlayerInfo;
		break;
	case 1:
		info = &SokuLib::rightPlayerInfo;
		break;
	case 2:
		info = &assists.first;
		break;
	case 3:
		info = &assists.second;
		break;
	}

	size_t size = sizeof(PacketLoadingReady) + info->effectiveDeck.size() * sizeof(unsigned short);
	char *buffer = new char[size];
	auto *packet = (PacketLoadingReady *)buffer;

	packet->type = LOADING_READY;
	packet->stage = SokuLib::gameParams.stageId;
	packet->music = SokuLib::gameParams.musicId;
	packet->skinId = info->palette;
	packet->deckId = info->deck;
	packet->hasSimulButtons = profiles[0]->hasSimulButtonsOff;
	packet->chr = static_cast<SokuLib::CharacterPacked>(info->character);
	packet->deckSize = info->effectiveDeck.size();
	for (size_t i = 0; i < info->effectiveDeck.size(); i++)
		packet->deck[i] = info->effectiveDeck[i];
	SokuLib::DLL::ws2_32.sendto(netobj.netUdp.socket, (char *)packet, size, 0, (sockaddr *)&netobj.netUdp.opponent, sizeof(netobj.netUdp.opponent));
	delete[] buffer;
}

void parseExtraChrsGameMatch(SokuLib::PlayerMatchData *ptr)
{
	auto *infos = &assists.first;

	loadLog("parseExtraChrsGameMatch: entered");

	for (int j = 0; j < 2; j++) {
		infos->character = static_cast<SokuLib::Character>(ptr->character);
		infos->palette = ptr->skinId;
		infos->deck = ptr->deckId;
		infos->effectiveDeck.clear();
		for (unsigned i = 0; i < ptr->deckSize; i++)
			infos->effectiveDeck.push_back(ptr->cards[i]);
		ptr = (SokuLib::PlayerMatchData *)ptr->getEndPtr();
		infos++;
	}
	loadLog("parseExtraChrsGameMatch: P3/P4 taken from the match packet");
}

void __declspec(naked) parseExtraChrsGameMatch_hook()
{
	__asm {
		PUSH ESI
		CALL parseExtraChrsGameMatch
		POP ESI
		MOVZX EAX, byte ptr [ESI]
		MOV byte ptr [EBX + 0x3a], AL
		RET
	}
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	DWORD old;

#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	const uint8_t versionString2v2[] = {
		0x41, 0xD0, 0x3B, 0x30, 0x64, 0x41, 0x74, 0xC8,
		0xC6, 0x24, 0x8C, 0xA4, 0x15, 0x44, 0x32, 0x96
	};

	myModule = hMyModule;
	GetModuleFileName(hMyModule, modFolder, 1024);
	PathRemoveFileSpec(modFolder);
	puts("Hello");
	// DWORD old;
	loadSoku2Config();
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_originalBattleMgrOnRender          = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender,    CBattleManager_OnRender);
	s_originalBattleMgrOnProcess         = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess,   CBattleManager_OnProcess);
	s_originalSelectOnProcess            = SokuLib::TamperDword(&SokuLib::VTable_Select.update,             CSelect_OnProcess);
	s_originalSelectOnRender             = SokuLib::TamperDword(&SokuLib::VTable_Select.onRender,           CSelect_OnRender);
	s_originalSelectClientOnProcess      = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.update,       CSelectCL_OnProcess);
	s_originalSelectClientOnRender       = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onRender,     CSelectCL_OnRender);
	s_originalTitleOnProcess             = SokuLib::TamperDword(&SokuLib::VTable_Title.onRender,            CTitle_OnProcess);
	s_originalCProfileDeckEdit_OnProcess = SokuLib::TamperDword(&SokuLib::VTable_ProfileDeckEdit.onProcess, CProfileDeckEdit_OnProcess);
	s_originalCProfileDeckEdit_OnRender  = SokuLib::TamperDword(&SokuLib::VTable_ProfileDeckEdit.onRender,  CProfileDeckEdit_OnRender);
	s_originalCProfileDeckEdit_Destructor= SokuLib::TamperDword(&SokuLib::VTable_ProfileDeckEdit.onDestruct,CProfileDeckEdit_Destructor);
	s_ogHudRender = (int (__thiscall *)(void *))SokuLib::TamperDword(0x85b544, onHudRender);
	memcpy((void *)0x858B80, versionString2v2, sizeof(versionString2v2));
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	// Force deck icon to be hidden in character select
	*(unsigned char *)0x4210e2 = 0xEB;
	// Force deck icon to be hidden in deck construction
	memset((void *)0x0044E4ED, 0x90, 35);
	SokuLib::TamperNearJmpOpr(0x450230, CProfileDeckEdit_SwitchCurrentDeck);
	s_originalDrawGradiantBar = reinterpret_cast<void (*)(float, float, float)>(SokuLib::TamperNearJmpOpr(0x44E4C8, drawGradiantBar));
	og_CProfileDeckEdit_Init = SokuLib::union_cast<SokuLib::ProfileDeckEdit *(SokuLib::ProfileDeckEdit::*)(int, int, SokuLib::Sprite *)>(
		SokuLib::TamperNearJmpOpr(0x0044d529, CProfileDeckEdit_Init)
	);

	new SokuLib::Trampoline(0x435377, onProfileChanged, 7);
	new SokuLib::Trampoline(0x450121, onDeckSaved, 6);

	SokuLib::TamperNearJmp(0x4080B2, saveProfiles);

	ogBattleMgrHandleCollision = SokuLib::TamperNearJmpOpr(0x47d618, CBattleManager_HandleCollision);
	SokuLib::TamperNearJmpOpr(0x47d64c, CBattleManager_HandleCollision);
	// Enable 4 characters collision
	*(char *)0x47D1A1 = 0x4;
	*(char *)0x47D520 = 0x4;
	const unsigned char collisionPatch[] = {
		// MOV AL, [ESP+0x28]
		0x8A, 0x44, 0x24, 0x28,
		// CMP AL, 2
		0x3C, 0x02,
		// JNE 47D530
		0x75, 0x04,
		// SUB ESI, 0x18
		0x83, 0xEE, 0x18,
		// NOP
		0x90
	};
	memcpy((void*)0x47D524, collisionPatch, sizeof(collisionPatch));
	// JNZ 47D530 -> JNZ 47D524
	*(char *)0x47D5BF = 0x61;

	// Enable 4 characters inputs
	*(char *)0x48219D = 0x4;
	// Disable inputs for all 4 characters in transitions
	*(char *)0x479714 = 0x4;

	const unsigned char chrSelectInputInitPatch[] = {
		// MOV [00898684],EBX
		0x89, 0x1D, 0x84, 0x86, 0x89, 0x00,
		// NOP
		0x90,
		// MOV EBX, 00898678
		0xBB, 0x78, 0x86, 0x89, 0x00,
		// mov [ebx],FEFEFEFE
		0xC7, 0x03, 0xFE, 0xFE, 0xFE, 0xFE,
		// XOR EBX, EBX
		0x31, 0xDB
	};
	memcpy((void*)0x43EA13, chrSelectInputInitPatch, sizeof(chrSelectInputInitPatch));

	new SokuLib::Trampoline(0x4796EE, updateOtherHud, 5);

	// Extra key in key config
	*(char *)0x4512AC = 0x8;
	*(char *)0x4511EA = 0xC;
	*(char *)0x45135A = 0xC;
	*(char *)0x4513A7 = 0xC;
	*(char *)0x451650 = 0xC;
	*(char *)0x4514DC = 0xC;

	og_switchBattleStateBreakAddr = (void *)SokuLib::TamperNearJmpOpr(0x481F58, healExtraChrs);
	og_advanceFrame = SokuLib::TamperNearJmpOpr(0x48535D, checkWakeUp);
	*(char *)0x481AAE = 0x8B;
	*(char *)0x481AAF = 0xC8;
	SokuLib::TamperNearCall(0x481AB0, onDeath);
	*(char *)0x47D6AC = 0;
	*(char *)0x47D6B8 = 0;

	const int callSites[] = {
		0x480970, 0x4809D0, 0x480A7B, 0x480B11, 0x480B95, 0x480DCD,
		0x48279A, 0x4827EA, 0x48283A, 0x4828AB, 0x482928, 0x4829AF
	};
	for (auto addr : callSites)
		SokuLib::TamperNearJmpOpr(addr, updateCollisionBoxes);

	SokuLib::TamperNearJmp(0x47B07C, setHealthRegen);
	*(char *)0x47B081 = 0x90;
	*(char *)0x47B082 = 0x90;
	new SokuLib::Trampoline(0x47D6E0, addHealthRegen, 7);
	*(char *)0x47D6E5 = 0x90;
	*(char *)0x47D6E6 = 0x90;
	new SokuLib::Trampoline(0x47D18E, clearHealthRegen, 7);
	*(char *)0x47D193 = 0x90;
	*(char *)0x47D194 = 0x90;

	new SokuLib::Trampoline(0x4209F6, onChrSelectComplete, 6);
	new SokuLib::Trampoline(0x42151E, onStageSelectCancel_hook, 6);

	SokuLib::TamperNearCall(0x43890E, getOgHud);
	*(char *)0x438913 = 0x90;

	new SokuLib::Trampoline(0x46DEC1, saveOldHud, 5);
	new SokuLib::Trampoline(0x46DEC6, swapStuff, 5);
	new SokuLib::Trampoline(0x46DF02, swapStuff, 5);
	new SokuLib::Trampoline(0x46DF44, swapStuff, 5);
	new SokuLib::Trampoline(0x46DF80, swapStuff, 5);
	new SokuLib::Trampoline(0x46DFC0, swapStuff, 5);
	SokuLib::TamperNearJmp(0x46E002, restoreOldHud);

	// On hit
	SokuLib::TamperNearJmpOpr(0x47B17C, turnAroundHitBlock_hook);
	// On rightblock
	SokuLib::TamperNearJmpOpr(0x47C2A3, turnAroundHitBlock_hook);
	// On wrongblock
	SokuLib::TamperNearJmpOpr(0x47C53C, turnAroundHitBlock_hook);

	new SokuLib::Trampoline(0x4442CD, onMenuTopProfileInit, 5);
	new SokuLib::Trampoline(0x4448C0, renderExtraProfilesTop, 7);
	*(char *)0x44CFEE = 0x50;
	SokuLib::TamperNearCall(0x44CFD8, selectProfileSpriteColor);

	og_loadDat = SokuLib::TamperNearJmpOpr(0x7fb85f, loadExtraDatFiles);

	// Chr select input stuff
	SokuLib::TamperNearCall(0x43E6C8, checkUsedInputs);
	memset((void *)(0x43E6C8 + 5), 0x90, 0x43E70B - 0x43E6C8 - 5);
	*(char *)0x43E6CD = 0x8A;
	*(char *)0x43E6CE = 0x86;
	*(char *)0x43E6CF = 0x78;
	*(char *)0x43E6D0 = 0x86;
	*(char *)0x43E6D1 = 0x89;
	*(char *)0x43E6D2 = 0x00;
	*(char *)0x43E6D3 = 0x3C;
	*(char *)0x43E6D4 = 0xFE;
	*(char *)0x43E6D5 = 0x75;
	*(char *)0x43E6D6 = 0x02;
	*(char *)0x43E6D7 = 0x5E;
	*(char *)0x43E6D8 = 0xC3;
	*(char *)0x43E72C = 0xB8;
	*(int  *)0x43E72D = 0x8986A8;
	SokuLib::TamperNearCall(0x43E731, setInputPointer);
	*(char *)0x43E736 = 0x90;
	SokuLib::TamperNearCall(0x43E723, setInputPointer);
	*(char *)0x43E728 = 0x90;
	*(char *)0x43E729 = 0x90;
	SokuLib::TamperNearCall(0x43E6A5, cmpInputPointer);
	*(char *)0x43E6AA = 0x90;
	*(char *)0x43E6AB = 0x90;
	*(char *)0x43E6AC = 0x90;
	SokuLib::TamperNearJmp(0x43E044, loadInputPointer);
	SokuLib::TamperNearCall(0x42289D, initExtraInputs_hook);
	SokuLib::TamperNearCall(0x422871, initExtraInputsLight_hook);

	new SokuLib::Trampoline(0x43EA0D, onCharacterSelectInit, 6);
	new SokuLib::Trampoline(0x42296C, chrSelect_pushActiveInputs, 5);

	og_SelectConstruct = SokuLib::TamperNearJmpOpr(0x41E55F, CSelect_construct);
	SokuLib::TamperNearJmpOpr(0x41E263, CSelect_construct);
	SokuLib::TamperNearJmpOpr(0x41E2C3, CSelect_construct);

	SokuLib::TamperNearCall(0x42112C, renderChrSelectChrDataGear_hook);
	*(char *)0x421131 = 0x90;
	SokuLib::TamperNearCall(0x420DAD, renderChrSelectChrData_hook);
	*(char *)0x420DB2 = 0x90;
	*(char *)0x420DB3 = 0x90;
	SokuLib::TamperNearCall(0x4212D9, renderChrSelectChrName_hook);
	*(char *)0x4212DE = 0x90;
	SokuLib::TamperNearCall(0x421350, renderChrSelectProfile_hook);
	*(char *)0x421355 = 0x90;

	SokuLib::TamperNearJmp(0x4208ED, updateCharacterSelect_hook);
	*(char *)0x4208F2 = 0x90;
	*(char *)0x4208F3 = 0x90;

	memset((void *)0x42081E, 0x90, 0x42085F - 0x42081E);
	SokuLib::TamperNearCall(0x420849, changePalette_hook0);
	memset((void *)0x4206DC, 0x90, 0x42072D - 0x4206DC);
	SokuLib::TamperNearCall(0x4206DC, changePalette_hook1);

	memset((void *)0x46012A, 0x90, 13);
	memset((void *)0x4622A4, 0x90, 13);

	*(char *)0x44D247 = 160;
	*(char *)0x44CC0E = 6;
	*(char *)0x44D227 = 6;
	*(char *)0x44CF61 = 6;
	*(char *)0x44CC44 = 5;
	*(char *)0x44CC5A = 0x47;
	*(int *)0x44CE64 = 0x44CC55;
	*(int *)0x44CE68 = 0x44CC55;
	*(int *)0x44CE6C = 0x44CC55;
	*(int *)0x44CE70 = 0x44CDF0;
	*(int *)0x44CE74 = 0x44CE47;
	SokuLib::TamperNearJmp(0x43E014, getProfileInfo);

	memset(extraProfiles, 0, sizeof(extraProfiles));
	*(int *)&profiles[2]->sprite = 0x8576AC;
	*(int *)&profiles[3]->sprite = 0x8576AC;
	SokuLib::TamperNearJmp(0x43E006, reloadProfiles);

	const unsigned char profileExtraInit[] = {
		// The top part here is exactly what the game did before but shorter in size to fit more assembly
		// Before the game did:
		// mov [esi+00000188],0x00000000
		// mov [esi+0000018C],0x00000001
		// ...
		0x31, 0xC0,                                                // xor eax,eax
		0x89, 0x86, 0x88, 0x01, 0x00, 0x00,                        // mov [esi+00000188],eax
		0x40,                                                      // inc eax
		0x89, 0x86, 0x8C, 0x01, 0x00, 0x00,                        // mov [esi+0000018C],eax
		0x40,                                                      // inc eax
		0x89, 0x86, 0x90, 0x01, 0x00, 0x00,                        // mov [esi+00000190],eax
		0x40,                                                      // inc eax
		0x89, 0x86, 0x94, 0x01, 0x00, 0x00,                        // mov [esi+00000194],eax
		0x40,                                                      // inc eax
		0x89, 0x86, 0x98, 0x01, 0x00, 0x00,                        // mov [esi+00000198],eax
		0x40,                                                      // inc eax
		0x89, 0x86, 0x9C, 0x01, 0x00, 0x00,                        // mov [esi+0000019C],eax
		0x40,                                                      // inc eax
		0x89, 0x86, 0xA0, 0x01, 0x00, 0x00,                        // mov [esi+000001A0],eax
		0x40,                                                      // inc eax
		0x89, 0x86, 0xA4, 0x01, 0x00, 0x00,                        // mov [esi+000001A4],eax
		0x90,                                                      // nop
		0x90,                                                      // nop
		0x90,                                                      // nop
		0xC7, 0x86, 0x70, 0x01, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00 // mov [esi+00000170],0000001C
	};
	memcpy((void *)0x435988, profileExtraInit, sizeof(profileExtraInit));

	SokuLib::TamperNearCall(0x42A554, getCharacterIndexResetHealth_hook);
	*(char *)0x42A559 = 0x90;
	*(char *)0x42A59D = 4;

	memset((void *)0x47B592, 0x90, 10);
	SokuLib::TamperNearCall(0x47B592, setGrazeFlags);

	// Proration bug fix
	memmove((void *)0x4799CD, (void *)0x4799D1, 0x4799F2 - 0x4799D1);
	*(unsigned *)0x4799EE = 0x0424448B;
	*(char *)0x47AF43 = 0x86; // fld dword ptr [eax+000004B0] -> fld dword ptr [esi+000004B0]

	const unsigned char computeProrationDamage[] = {
		// mov eax,[ecx+00000170] -> mov eax,[esi+0000016C]
		0x8B, 0x86, 0x6C, 0x01, 0x00, 0x00
	};
	memcpy((void *)0x463F59, computeProrationDamage, sizeof(computeProrationDamage));
	// fmul dword ptr [esi+000004B0] -> fmul dword ptr [eax+000004B0]
	*(char *)0x463F83 = 0x88;

	// mov eax,[edi+0000016C] -> mov eax,[esi+0000016C]
	*(char *)0x47B016 = 0x86;
	*(char *)0x47B057 = 0x86;
	*(char *)0x47B048 = 0x86;
	*(char *)0x47B004 = 0x86;
	*(char *)0x47AB70 = 0x86;

	*(char *)0x464A82 = 0x51;
	SokuLib::TamperNearCall(0x464A83, 0x463F50);
	*(char *)0x464A88 = 0x5E;

	// We swap the left and right combo displays
	// Original code:
	//0047e2c0 8b 44 24 14     MOV        EAX,dword ptr [ESP + 0x14]=>local_10
	//0047e2c4 8b 4c 24 18     MOV        ECX,dword ptr [ESP + 0x18]=>local_c
	//0047e2c8 8d 04 b0        LEA        EAX,[EAX + ESI*0x4]
	//0047e2cb 03 05 e4        ADD        EAX,dword ptr [CBattleManagerPtr]
	//         85 89 00
	//0047e2d1 8b 44 04 1c     MOV        EAX,dword ptr [ESP + EAX*0x1 + 0x1c]
	//0047e2d1 8b 44 04 1c     MOV        EAX,dword ptr [ESP + EAX*0x1 + 0x1c]
	//0047e2d5 50              PUSH       EAX
	//0047e2d6 51              PUSH       ECX
	//0047e2d7 8b cf           MOV        ECX,EDI
	//0047e2d9 89 44 b4 24     MOV        dword ptr [ESP + ESI*0x4 + 0x24]=>local_4,EAX
	memset((void *)0x47E2C0, 0x90, 0x47E2DD - 0x47E2C0);
	SokuLib::TamperNearCall(0x47E2C4, swapComboCtrs);

	// Netplay support
	SokuLib::TamperNearCall(0x4547AA, copyExtraProfileNames);
	*(char *)0x4547AF = 0x90;

	// PUSH 0
	// CALL reloadNetProfileTexture
	// PUSH 1
	// CALL reloadNetProfileTexture
	// PUSH 2
	// CALL reloadNetProfileTexture
	// PUSH 3
	// CALL reloadNetProfileTexture
	// ADD ESP, 16
	// JMP 0x453E53
	*(unsigned short *)0x453B8D = 0x006A;
	SokuLib::TamperNearCall(0x453B8F, reloadNetProfileTexture);
	*(unsigned short *)0x453B94 = 0x016A;
	SokuLib::TamperNearCall(0x453B96, reloadNetProfileTexture);
	*(unsigned short *)0x453B9B = 0x026A;
	SokuLib::TamperNearCall(0x453B9D, reloadNetProfileTexture);
	*(unsigned short *)0x453BA2 = 0x036A;
	SokuLib::TamperNearCall(0x453BA4, reloadNetProfileTexture);
	*(unsigned short *)0x453BA9 = 0xC483;
	*(unsigned char *)0x453BAB = 16;
	*(char *)0x453BAC = 0xE9;
	*(unsigned *)0x453BAD = 0x02A2;

	SokuLib::TamperNearCall(0x454D6A, inputMul4);
	*(unsigned short *)0x454D81 = 0xEB8B;
	*(unsigned int *)0x454D83 = 0x90909090;
	memset((void *)0x454D9B, 0x90, 0x454DD9 - 0x454D9B);
	// SUB ESI, 4
	// PUSH ESI
	// PUSH EBP
	// PUSH EDI
	// CALL pushOnlineInputs
	// ADD ESP, 12
	*(char *)0x454D9B = 0x83;
	*(char *)0x454D9C = 0xee;
	*(char *)0x454D9D = 0x04;
	*(char *)0x454D9E = 0x56;
	*(char *)0x454D9F = 0x55;
	*(char *)0x454DA0 = 0x57;
	SokuLib::TamperNearCall(0x454DA1, pushOnlineInputs);
	*(unsigned short *)0x454DA6 = 0xC483;
	*(unsigned char *)0x454DA8 = 12;

	SokuLib::TamperNearJmpOpr(0x454BEA, updateExtraInputHandlers);
	SokuLib::TamperNearCall(0x4287DF, updateExtraOnlineInputs);

	*(void **)0x41E010 = handlePlayerJoinPacket_hook;
	*(void **)0x41E014 = handlePlayerJoinAckPacket_hook;
	*(void **)0x41E01C = handleUnlockCharSelectPacket_hook;

	new SokuLib::Trampoline(0x428789, sendCurrentSelectCharacter, 5);
	SokuLib::TamperNearCall(0x45409A, parseExtraChrsGameMatch_hook);
	*(char *)0x45409F = 0x90;
	og_FUNCALL43F996 = SokuLib::TamperNearJmpOpr(0x43F996, onInputsReset);

	// Return SELECTCL, instead of SELECTSV
	*(char *)0x4286C5 = SokuLib::SCENE_SELECTCL;

	// Enable select key
	*(unsigned short *)0x454CD6 = 0b101111111111;
	*(unsigned short *)0x454CC2 = 0b101111111111;

	// Different asset paths
	*(const char **)0x47DC8E = effectPath;
	*(const char **)0x47DEC2 = effectPath;
	*(const char **)0x47DEE5 = battleUpperPath1;
	*(const char **)0x47DEFD = battleUnderPath1;
	*(const char **)0x444294 = menuProfileXml;
	*(const char **)0x44D1FA = menuSelectXml;
	*(const char **)0x4248CB = characterSelectXml;

	og_handleInputs = SokuLib::TamperNearJmpOpr(0x48224D, handlePlayerInputs);
	s_origLoadDeckData = SokuLib::TamperNearJmpOpr(0x437D23, loadDeckData);
	og_FUN4098D6 = SokuLib::TamperNearJmpOpr(0x43F136, loadExtraPlayerInputs);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}