/* Headless diagnostic: real native bone decoder and hierarchy transform.
 * Reads game-owned blobs supplied by the user; never modifies those files. */
#include "pe1/render_object.h"
#include "pe1/gte.h"
#include "pe1/game_state.h"
#include "pe1/render_packets.h"
#include "pe1/render_setup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
static uint32_t rd(const unsigned char *p) { return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24; }
static unsigned char *load(const char *path,size_t *n) {
 FILE *f=fopen(path,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long len=ftell(f);rewind(f);
 if(len<8 || len>16*1024*1024){fclose(f);return NULL;}
 unsigned char *p=malloc(len);if(!p){fclose(f);return NULL;}*n=fread(p,1,len,f);fclose(f);if(*n!=(size_t)len){free(p);return NULL;}return p;
}
static int row_length(const unsigned char *p,size_t n) {
 if(n<12)return 0;size_t at=12;unsigned frames=p[2], sh=(p[0]&3)==2;
 for(unsigned c=0;c<3+(p[1]+1u)*3;++c){
  if(at+4>n)return 0;
  int wide=c<3 || sh;unsigned marker=wide?(p[at]|p[at+1]<<8):p[at];
  size_t bytes=marker?4:(((frames>>(wide?1:2))+1u)*4u);
  if(at+bytes>n)return 0;at+=bytes;
 }return (int)at;
}
static RenderMatrix identity={.rotation={{4096,0,0},{0,4096,0},{0,0,4096}}};
void Psyz_PgxpSetActive(int active);
static unsigned rows,frames_total,warnings;
static void sweep(RenderObjectEntity *obj,unsigned char *row,size_t size,unsigned id,FILE *log) {
 int length=row_length(row,size);unsigned bones=size>=2?row[1]+1u:0;
 if(!length || bones>obj->header->part_count){fprintf(log,"%u,-1,%u,%u,0,0,0,invalid_row\n",id,bones,obj->header->part_count);warnings++;return;}
 rows++;unsigned frames=row[2]?row[2]:1;unsigned hashes[256]={0};
 for(unsigned part=0;part<obj->header->part_count;++part)obj->matrices[part]=identity;
 for(unsigned pass=0;pass<2;++pass) for(unsigned frame=0;frame<frames;++frame){
  Psyz_PgxpNextFrame();
  obj->model_matrix=identity;
  Anim_BuildRotationMatrices(obj,(RenderAnimationDataHeader *)row,frame);
  Render_TransformVertices(obj);
  long long max_translation=0;unsigned hash=2166136261u;
  for(unsigned p=0;p<obj->header->part_count;++p){
   unsigned char *b=(unsigned char *)&obj->matrices[p];
   for(unsigned j=0;j<sizeof(RenderMatrix);++j)if(j<18 || j>=20)hash=(hash^b[j])*16777619u;
   for(int k=0;k<3;++k){long long v=obj->matrices[p].translation[k];if(v<0)v=-v;if(v>max_translation)max_translation=v;}
  }
  unsigned repeat_changed=pass && hashes[frame]!=hash;
  if(!pass)hashes[frame]=hash;
  const char *status=bones<obj->header->part_count?"missing_bones":repeat_changed?"repeat_changed":max_translation>10000?"large_translation":"ok";
  if(strcmp(status,"ok"))warnings++;
  unsigned faces[4]={0};
  GteMatrix view={{{4096,0,0},{0,4096,0},{0,0,4096}},{0,0,2000}};
  gte_SetGeomOffset(160,112);gte_ctc2_26(256);gte_lddqa(0);gte_lddqb(0);
  memset(g_RenderClutLookupTable,0,sizeof(u32)*65541);
  memset(g_RenderVertexWorkWords,0,sizeof(u32)*65541);
  Render_TransformMorphVertices(obj,(u32 *)&view);
  unsigned counts[]={obj->header->packet34_count,obj->header->packet28_count,obj->header->packet24_count,obj->header->packet1c_count};
  size_t sizes[]={sizeof(RenderPacket34),sizeof(RenderPacket28),sizeof(RenderPacket24),sizeof(RenderPacket1C)},bytes=0;
  for(int c=0;c<4;++c)bytes+=counts[c]*2*sizes[c];
  memset(obj->primitive_buffer,0,bytes);
  static RenderNativeTag ordering[4096];
  for(int i=0;i<4096;++i)ordering[i].address=1;
  D_800B0E38.ordering[0]=ordering;g_ActiveDrawSlot=0;
  int depth_ok=1;
  for(unsigned p=0;p<obj->header->vertex_count;++p)if(D_800A636C[p]<0 || D_800A636C[p]>16000)depth_ok=0;
  if(depth_ok){
   Render_DrawTexturedQuads(obj);
   unsigned char *pk=obj->primitive_buffer;
   for(int c=0;c<4;++c)for(unsigned i=0;i<counts[c];++i,pk+=2*sizes[c])faces[c]+=((RenderNativeTag *)pk)->address!=0;
  }
  fprintf(log,"%u,%u,%u,%u,%u,%u,%lld,%s,%u,%u,%u,%u\n",id,pass*frames+frame,bones,obj->header->part_count,frame,hash,max_translation,status,faces[0],faces[1],faces[2],faces[3]);frames_total++;
 }
}
int main(int argc,char **argv) {
 if(argc!=4){fprintf(stderr,"Usage: pe-animation-sweep model-container animation-container output.csv\n");return 2;}
 Psyz_PgxpSetActive(getenv("PE_SWEEP_PGXP") != NULL);
 size_t mn,an;unsigned char *m=load(argv[1],&mn),*a=load(argv[2],&an);FILE *log=NULL;int result=2;
 if(!m||!a){fprintf(stderr,"Cannot read input\n");goto done;}
 uint32_t dir=rd(m+4);if(dir>mn || mn-dir<20)goto done;
 uint32_t roots=rd(m+dir+12),tab=roots&0x3fffff;
 if(!(roots>>22)||tab>mn||mn-tab<12)goto done;
 uint32_t model=rd(m+tab+4)&0xffffff;if(model>mn||mn-model<28)goto done;
 RenderObjectEntity obj={0};obj.header=(RenderObjectHeader *)(m+model);
 unsigned np=obj.header->part_count,nv=obj.header->vertex_count;
 size_t desc=28+12*np+12*nv, packets=obj.header->packet34_count+obj.header->packet28_count+obj.header->packet24_count+obj.header->packet1c_count;
 size_t bounds=desc+12*packets+16,commands=bounds+16*np+8;
 if(!np||np>128||commands+((obj.header->matrix_command_bytes+3u)&~3u)+16u*obj.header->packet34_count+12u*obj.header->packet28_count>mn-model)goto done;
 RenderObjectPart *validated_parts=(RenderObjectPart *)(m+model+28);
 for(unsigned p=0;p<np;++p)if((unsigned)validated_parts[p].vertex_start+validated_parts[p].vertex_count>nv)goto done;
 RenderPrimitiveDescriptor *validated_faces=(RenderPrimitiveDescriptor *)(m+model+desc);
 for(size_t p=0;p<packets;++p){
  int quad=p<obj.header->packet34_count || (p>=(size_t)obj.header->packet34_count+obj.header->packet28_count && p<(size_t)obj.header->packet34_count+obj.header->packet28_count+obj.header->packet24_count);
  for(int v=0;v<(quad?4:3);++v)if(validated_faces[p].lookup_indices[v]>=nv)goto done;
 }
 obj.vertices=(RenderVec3s *)(m+model+28+12*np);
 obj.primitive_descriptors=(RenderPrimitiveDescriptor *)(m+model+desc);
 obj.primitive_buffer=calloc(2,obj.header->packet34_count*sizeof(RenderPacket34)+obj.header->packet28_count*sizeof(RenderPacket28)+obj.header->packet24_count*sizeof(RenderPacket24)+obj.header->packet1c_count*sizeof(RenderPacket1C)+1);
 if(!obj.primitive_buffer)goto done;
 obj.parts=(RenderObjectPart *)(m+model+28);obj.bounds_vertices=(RenderVec3s *)(m+model+bounds);
 obj.matrix_commands=(s8 *)(m+model+commands);obj.matrices=calloc(np,sizeof(RenderMatrix));obj.animation_entries=calloc(np,sizeof(RenderAnimationLookupEntry));
 if(!obj.matrices||!obj.animation_entries)goto done;
 printf("model parts=%u primitives=%zu native_render_bytes=%zu\n",np,packets,2*(obj.header->packet34_count*sizeof(RenderPacket34)+obj.header->packet28_count*sizeof(RenderPacket28)+obj.header->packet24_count*sizeof(RenderPacket24)+obj.header->packet1c_count*sizeof(RenderPacket1C))+obj.header->animation_entry_count*sizeof(RenderAnimationLookupEntry)+np*sizeof(RenderMatrix));
 /* Exercise native primitive setup as well as decoding; detect header counts
  * that underestimate the matrix/lookup storage the renderer actually writes. */
 size_t packet_bytes=2*(obj.header->packet34_count*sizeof(RenderPacket34)+obj.header->packet28_count*sizeof(RenderPacket28)+obj.header->packet24_count*sizeof(RenderPacket24)+obj.header->packet1c_count*sizeof(RenderPacket1C));
 size_t declared=packet_bytes+obj.header->animation_entry_count*sizeof(RenderAnimationLookupEntry)+np*sizeof(RenderMatrix);
 unsigned char *probe_memory=calloc(1,declared+np*sizeof(RenderAnimationLookupEntry)+4096);
 if(!probe_memory)goto object_done;
 RenderObjectEntity probe={0};s8 *uvs=NULL;
 Render_SetupEntityPrims(&probe,obj.header,probe_memory,0,0,0,0,0,&uvs,1);
 size_t used=(unsigned char *)(probe.matrices+np)-probe_memory;
 printf("native_setup_declared=%zu native_setup_used=%zu status=%s\n",declared,used,used>declared?"allocation_underestimated":"ok");
 if(used>declared)warnings++;
 free(probe_memory);
 unsigned visited=0;int depth=0;
 for(unsigned i=0;i<obj.header->matrix_command_bytes;++i){int c=obj.matrix_commands[i];if(c==-1){if(++depth>=32)goto object_done;}else if(c==-2){if(--depth<0)goto object_done;}else if(c<0||(unsigned)c!=visited++||visited>np)goto object_done;}
 if(visited!=np||depth)goto object_done;
 log=fopen(argv[3],"w");if(!log)goto object_done;
 fprintf(log,"row,sample,bones,parts,frame,matrix_hash,max_translation,status,faces0,faces1,faces2,faces3\n");
 if(!memcmp(a,"PERR",4)){
  unsigned n=rd(a+4);if(n>(an-8)/12)goto object_done;
  for(unsigned i=0;i<n;++i){unsigned char *t=a+8+12*i;uint32_t off=rd(t+4),len=rd(t+8);if(off<=an&&len<=an-off)sweep(&obj,a+off,len,rd(t),log);else goto object_done;}
 }else{
  dir=rd(a+4);if(dir>an||an-dir<20)goto object_done;
  uint32_t entries=rd(a+dir+16),at=entries&0x3fffff,n=entries>>22;
  if(at>an||n>(an-at)/12)goto object_done;
  for(unsigned i=0;i<n;++i){uint32_t val=rd(a+at+12*i+4),off=val&0xffffff;if(off>=an)goto object_done;sweep(&obj,a+off,an-off,val>>24,log);}
 }
 printf("%s + %s: %u rows, %u frame samples, %u warnings\n",argv[1],argv[2],rows,frames_total,warnings);result=0;
object_done:free(obj.primitive_buffer);free(obj.matrices);free(obj.animation_entries);
done:if(log)fclose(log);free(m);free(a);return result;
}
