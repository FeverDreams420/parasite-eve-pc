#ifndef PE1_AKAO_STREAM_H
#define PE1_AKAO_STREAM_H

/* SPU sample transfers and streamed instrument uploads. */
void Spu_ClearTransferCallback(void);
void Spu_PrepareTransfer(void);
void Spu_UploadWithPrepare(void *source, unsigned int size);
void Spu_ReadWithPrepare(void *destination, unsigned int size);
void Spu_WaitTransferDone(void);
int Spu_UploadSampleBlock(int *header, int wait);
void Spu_InitHardware(void);
void Spu_SetSampleTableBase(unsigned short *table);
void Spu_Shutdown(void);
void Spu_RebaseStreamAddrs(unsigned char *instruments, int address, int count);
int Spu_UploadStreamBlockA(unsigned bank, unsigned char *cursor, unsigned size);
int Spu_UploadStreamBlockB(unsigned bank, unsigned char *cursor);
int Akao_StepNoteSequencer(unsigned char *cursor, unsigned size);
int Spu_SetStreamModeA(void);
int Spu_SetStreamModeB(void);
int Spu_GetTransferStatus(void);
int Spu_UploadSampleBlockBlocking(void *block, int mode);

#endif
