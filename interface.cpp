
#include <sys/stat.h>
#include <math.h>

#include "../interface.h"
#include "snes9x.h"
#include "memmap.h"
#include "display.h"
#include "controls.h"
#include "apu.h"
#include "cpuexec.h"
#include "controls.h"

void module_init(const module_init_info_t *init_info, module_info_t *module_info)
{
   memset(&Settings, 0, sizeof(Settings));
   Settings.MouseMaster = TRUE;
   Settings.SuperScopeMaster = TRUE;
   Settings.JustifierMaster = TRUE;
   Settings.MultiPlayer5Master = TRUE;
   Settings.FrameTimePAL = 20000;
   Settings.FrameTimeNTSC = 16667;
   Settings.SixteenBitSound = TRUE;
   Settings.Stereo = TRUE;
   Settings.SoundPlaybackRate = 32000;
   Settings.SoundInputRate = 32000;
   Settings.SupportHiRes = TRUE;
   Settings.Transparency = TRUE;
   Settings.AutoDisplayMessages = TRUE;
   Settings.InitialInfoStringTimeout = 120;
   Settings.HDMATimingHack = 100;
   Settings.BlockInvalidVRAMAccessMaster = TRUE;
   Settings.StopEmulation = TRUE;
   Settings.WrongMovieStateProtection = TRUE;
   Settings.DumpStreamsMaxFrames = -1;
   Settings.StretchScreenshots = 1;
   Settings.SnapshotScreenshots = TRUE;
   Settings.SkipFrames = AUTO_FRAMERATE;
   Settings.TurboSkipFrames = 15;
   Settings.CartAName[0] = 0;
   Settings.CartBName[0] = 0;

   CPU.Flags = 0;

   Memory.Init();
   GFX.Pitch = 512;
   S9xGraphicsInit();
   S9xSetRenderPixelFormat(RGB565);
   S9xInitAPU();
   S9xInitSound(20, 0);

   if (!Memory.LoadROM(init_info->filename))
   {
      debug_log("load error\n");
      return;
   }

   module_info->output_width = 256;
   module_info->output_height = 224;
   module_info->screen_format = screen_format_RGB565;
   module_info->stereo = true;
   module_info->framerate = 60;
   module_info->audio_rate = Settings.SoundInputRate;

   S9xMapButton(PAD_BUTTON_A, S9xGetCommandT("Joypad1 A"), false);
   S9xMapButton(PAD_BUTTON_B, S9xGetCommandT("Joypad1 B"), false);
   S9xMapButton(PAD_BUTTON_X, S9xGetCommandT("Joypad1 X"), false);
   S9xMapButton(PAD_BUTTON_Y, S9xGetCommandT("Joypad1 Y"), false);
   S9xMapButton(PAD_BUTTON_UP, S9xGetCommandT("Joypad1 Up"), false);
   S9xMapButton(PAD_BUTTON_DOWN, S9xGetCommandT("Joypad1 Down"), false);
   S9xMapButton(PAD_BUTTON_LEFT, S9xGetCommandT("Joypad1 Left"), false);
   S9xMapButton(PAD_BUTTON_RIGHT, S9xGetCommandT("Joypad1 Right"), false);
   S9xMapButton(PAD_BUTTON_L, S9xGetCommandT("Joypad1 L"), false);
   S9xMapButton(PAD_BUTTON_R, S9xGetCommandT("Joypad1 R"), false);
   S9xMapButton(PAD_BUTTON_SELECT, S9xGetCommandT("{Joypad1 Select,Mouse1 L}"), false);
   S9xMapButton(PAD_BUTTON_START, S9xGetCommandT("{Joypad1 Start,Mouse1 R}"), false);

   debug_log("module init\n");
}

void module_destroy()
{
   S9xGraphicsDeinit();
   debug_log("module destroy\n");
}

bool8 S9xDeinitUpdate(int width, int height)
{
   return TRUE;
}

void module_run(module_run_info_t *run_info)
{
   GFX.Screen = run_info->screen.u16;
   //   GFX.Pitch = run_info->pitch;
   //   GFX.ScreenSize = GFX.Pitch / 2 * SNES_HEIGHT_EXTENDED * (Settings.SupportHiRes ? 2 : 1);
   //   GFX.PPL = GFX.RealPPL = GFX.Pitch >> 1;

   int i;
   for(i = 0; i < PAD_BUTTON_MAX; i++)
      S9xReportButton(i, run_info->pad->buttons.mask & (1 << i));

//   if(run_info->pad->buttons.A)
//      debug_log("A PRESSED!!\n");
//   else
//      debug_log("A RELEASED!!\n");
   S9xMainLoop();
   S9xFinalizeSamples();
   run_info->max_samples = S9xGetSampleCount() >> 1;
   S9xMixSamples(run_info->sound_buffer.u8, run_info->max_samples << 1);
//   int i;
//   static float phase = 0;
//   float freq = 4000;
//   for(i=0; i < run_info->max_samples; i++)
//   {
//      run_info->sound_buffer.u16[i << 1] = sin(phase) * 0x4000;
//      run_info->sound_buffer.u16[(i << 1) + 1] = sin(phase) * 0x4000;
//      phase += M_2_PI * freq / Settings.SoundInputRate;
//   }

   run_info->frame_completed = true;
}
#ifndef __MINGW32__
void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
   *drive = 0;

   const char  *slash = strrchr(path, SLASH_CHAR),
                *dot   = strrchr(path, '.');

   if (dot && slash && dot < slash)
      dot = NULL;

   if (!slash)
   {
      *dir = 0;

      strcpy(fname, path);

      if (dot)
      {
         fname[dot - path] = 0;
         strcpy(ext, dot + 1);
      }
      else
         *ext = 0;
   }
   else
   {
      strcpy(dir, path);
      dir[slash - path] = 0;

      strcpy(fname, slash + 1);

      if (dot)
      {
         fname[dot - slash - 1] = 0;
         strcpy(ext, dot + 1);
      }
      else
         *ext = 0;
   }
}

void _makepath(char *path, const char *, const char *dir, const char *fname, const char *ext)
{
   if (dir && *dir)
   {
      strcpy(path, dir);
      strcat(path, SLASH_STR);
   }
   else
      *path = 0;

   strcat(path, fname);

   if (ext && *ext)
   {
      strcat(path, ".");
      strcat(path, ext);
   }
}
#endif

void S9xMessage(int type, int number, const char *message)
{
   const int   max = 36 * 3;
   static char buffer[max + 1];

   debug_log("%s\n", message);
   strncpy(buffer, message, max + 1);
   buffer[max] = 0;
   S9xSetInfoString(buffer);
}

bool S9xPollButton(uint32 id, bool *pressed)
{
   return 0;
}
bool8 S9xInitUpdate(void)
{
   return TRUE;
}

bool8 S9xContinueUpdate(int width, int height)
{
   return TRUE;
}

void S9xSetPalette(void)
{
}

void S9xAutoSaveSRAM(void)
{
   Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR));
}

static const char dirNames[13][32] =
{
   "",            // DEFAULT_DIR
   "",            // HOME_DIR
   "",            // ROMFILENAME_DIR
   "rom",         // ROM_DIR
   "sram",        // SRAM_DIR
   "savestate",   // SNAPSHOT_DIR
   "screenshot",  // SCREENSHOT_DIR
   "spc",         // SPC_DIR
   "cheat",    // CHEAT_DIR
   "patch",    // PATCH_DIR
   "bios",        // BIOS_DIR
   "log",         // LOG_DIR
   ""
};

const char *s9x_base_dir = ".";

const char *S9xGetDirectory(enum s9x_getdirtype dirtype)
{
   static char s[PATH_MAX + 1];

   if (dirNames[dirtype][0])
      snprintf(s, PATH_MAX + 1, "%s%s%s", s9x_base_dir, SLASH_STR, dirNames[dirtype]);
   else
   {
      switch (dirtype)
      {
      case DEFAULT_DIR:
         strncpy(s, s9x_base_dir, PATH_MAX + 1);
         s[PATH_MAX] = 0;
         break;

      case HOME_DIR:
         strncpy(s, getenv("HOME"), PATH_MAX + 1);
         s[PATH_MAX] = 0;
         break;

      case ROMFILENAME_DIR:
         strncpy(s, Memory.ROMFilename, PATH_MAX + 1);
         s[PATH_MAX] = 0;

         for (int i = strlen(s); i >= 0; i--)
         {
            if (s[i] == SLASH_CHAR)
            {
               s[i] = 0;
               break;
            }
         }

         break;

      default:
         s[0] = 0;
         break;
      }
   }

   return (s);
}


const char *S9xGetFilename(const char *ex, enum s9x_getdirtype dirtype)
{
   static char s[PATH_MAX + 1];
   char     drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

   _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
   snprintf(s, PATH_MAX + 1, "%s%s%s%s", S9xGetDirectory(dirtype), SLASH_STR, fname, ex);

   return (s);
}

bool8 S9xOpenSnapshotFile(const char *filename, bool8 read_only, STREAM *file)
{
   char  s[PATH_MAX + 1];
   char  drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

   _splitpath(filename, drive, dir, fname, ext);

   if (*drive || *dir == SLASH_CHAR || (strlen(dir) > 1 && *dir == '.' && *(dir + 1) == SLASH_CHAR))
   {
      strncpy(s, filename, PATH_MAX + 1);
      s[PATH_MAX] = 0;
   }
   else
      snprintf(s, PATH_MAX + 1, "%s%s%s", S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR, fname);

   if (!*ext && strlen(s) <= PATH_MAX - 4)
      strcat(s, ".frz");

   if ((*file = OPEN_STREAM(s, read_only ? "rb" : "wb")))
      return (TRUE);

   return (FALSE);
}

void S9xCloseSnapshotFile(STREAM file)
{
   CLOSE_STREAM(file);
}

const char *S9xBasename(const char *f)
{
   const char  *p;

   if ((p = strrchr(f, '/')) != NULL || (p = strrchr(f, '\\')) != NULL)
      return (p + 1);

   return (f);
}

bool8 S9xOpenSoundDevice(void)
{
   return TRUE;
}

const char *S9xGetFilenameInc(const char *ex, enum s9x_getdirtype dirtype)
{
   static char s[PATH_MAX + 1];
   char     drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

   unsigned int   i = 0;
   const char     *d;
   struct stat    buf;

   _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
   d = S9xGetDirectory(dirtype);

   do
      snprintf(s, PATH_MAX + 1, "%s%s%s.%03d%s", d, SLASH_STR, fname, i++, ex);

   while (stat(s, &buf) == 0 && i < 1000);

   return (s);
}
void S9xExit(void)
{
}
const char *S9xChooseFilename(bool8 read_only)
{
   return "";
}

const char *S9xChooseMovieFilename(bool8 read_only)
{
   return "";
}

void S9xToggleSoundChannel(int c)
{

}
const char *S9xStringInput(const char *message)
{
   return (NULL);
}

void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2)
{

}

bool S9xPollAxis(uint32 id, int16 *value)
{
   return true;
}

bool S9xPollPointer(uint32 id, int16 *x, int16 *y)
{
   return true;
}
void S9xSyncSpeed(void)
{

}
