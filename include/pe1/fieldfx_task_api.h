#ifndef PE1_FIELDFX_TASK_API_H
#define PE1_FIELDFX_TASK_API_H
/* Process dispatch only needs the opaque owner, not its renderer payload. */
struct FieldFxTaskOwner;
int FieldFx_BindTaskProgram(struct FieldFxTaskOwner *owner, int index);
#endif
