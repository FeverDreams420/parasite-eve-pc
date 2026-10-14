#include "pe1/akao.h"
#include "pe1/akao/globals.h"

/* Runtime storage for asset-backed original driver inputs. */
unsigned short *g_AkaoSampleOffsetTable;
char *g_AkaoSampleDataBase;
AkaoStagedCommand g_AkaoCmd;
AkaoSequenceCounter g_AkaoCmd24Sequence = {0x400};
AkaoQueueEntry g_AkaoMessageQueue[16];
int g_AkaoMessageQueueCount;
volatile u32 g_AkaoMessageQueueLock;
/* The current frontend submits only relative effect commands. Other message
 * handlers join this table as their original callers are integrated. */
AkaoMessageHandler Akao_MessageHandlers[256] = {[0x24] = Seq_StartRelativeNestedStream};
char g_AkaoDefaultVoiceProgram[4];
short g_AkaoPanVolumeTable[256];
AkaoU8 g_AkaoOpcodeArgLengths[96], g_AkaoExtOpcodeArgLengths[32];
u16 g_AkaoNoteLengthTable[11];
void *g_AkaoSoundEntryTable[256];
void *g_AkaoBgmHandle[2];
