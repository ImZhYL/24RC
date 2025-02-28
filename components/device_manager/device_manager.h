 #ifndef _DEVICE_MANAGER_H_
 #define _DEVICE_MANAGER_H_


void device_manager_init(void);


void resume_openmv(void);
void suspend_openmv(void);

void suspend_walk_opas(void);
void suspend_clamp_opas(void);

void resume_walk_opas(void);
void resume_clamp_opas(void);


#endif



