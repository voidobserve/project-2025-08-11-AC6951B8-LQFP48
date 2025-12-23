#ifndef __INSTRUCTION_FEEDBACK_H__
#define __INSTRUCTION_FEEDBACK_H__

#include "includes.h"

void instruction_feedback_buffer(u8* buffer, u8 len);

void instruction_feedback_success(u8 addr, u8 cmd_type);
void instruction_feedback_fail(u8 addr, u8 cmd_type);

#endif
