/* Psy-Q LIBPAD PADSEQD.OBJ: _padInitDirSeq, LIBPAD_PADSEQD_text_34, LIBPAD_PADSEQD_text_108, _dirFailAuto, CardObj_IsTransferActive. */

#include "pe1/card_obj.h"
#include "pe1/psyq_pad_main.h"

int LIBPAD_PADSEQD_text_34(CardObj *obj);
int CardObj_IsTransferActive(CardObj *obj);
void LIBPAD_PADSEQD_text_108(CardObj *port);

void _padInitDirSeq(void) {
    void (*response_handler)(CardObj *);

    g_MemCardStateDispatchFn = LIBPAD_PADSEQD_text_34;
    g_MemCardIsTransferActiveFn = CardObj_IsTransferActive;
    response_handler = LIBPAD_PADSEQD_text_108;
    g_MemCardResponseHandler = response_handler;
}

void _padCmdParaMode(CardObj *obj, unsigned char value);
void _padSendAtLoadInfo(CardObj *obj);

int LIBPAD_PADSEQD_text_34(CardObj *obj) {
    int compare;
    int state;

    compare = 0xF3;
    if (obj->response_3c[0] == compare) {
        compare = obj->currentId;
        if (compare == 0) {
            goto emit_zero;
        }
    }

    state = obj->sequenceStep;
    compare = 1;
    if (state != compare) {
    compare = state < 2;
    if (compare != 0) {
    if (state == 0) {
        return 0;
    }
    goto dispatch;

    }
high_state:
    compare = 0xFE;
    if (state == compare) {
        goto emit_zero;
    }
    compare = 0xFF;
    if (state == compare) {
        return 0;
    }

    }
    else
    {
emit_one:
    _padCmdParaMode(obj, 1);
    return 0;

emit_zero:
    _padCmdParaMode(obj, 0);
    return 0;

    }
dispatch:
    if (obj->fn_14 != 0) {
        obj->fn_14(obj);
    } else {
        _padSendAtLoadInfo(obj);
    }

done:
    return 0;
}


int _padRecvAtLoadInfo(CardObj *);
void LIBPAD_PADSEQD_text_108(CardObj *port) {
    int old = port->currentId;
    int i;
    int mode = port->response_3c[0] >> 4;
    port->currentId = mode;
    if (mode == 15)
        port->currentId = old;
    else {
        port->output_30[0] = 0;
        port->output_30[1] = port->response_3c[0];
        for (i = 2; i < port->response_index; i++)
            port->output_30[i] = port->response_3c[i];
    }
    if ((port->response_3c[1] == 0 && (port->sequenceStep != 1 || port->fn_14 != 0) &&
        port->pad_50[0] == 0) ||
        (!CardObj_IsTransferActive(port) && !port->saved_command && !port->retryCount &&
        port->currentId != old))
        g_MemCardObjResetFn(port);
    port->retryCount = 0;
    if ((u8)(port->sequenceStep - 2) < 252 && port->response_3c[0] != 0xf3)
        g_MemCardObjResetFn(port);
    {
        int state = port->sequenceStep;
        int one;
        int value;
        if (state != 0 && state != 255 && !port->command)
            return;
        one = 1;
        if (state != one) {
        if (state < 2) {
            value = 254;
            if (state == 0)
                goto idle;
            goto advance;
        }
        value = 254;
        if (state == value)
            goto terminal;
        if (state == 255)
            return;
        goto advance;
    idle:
        if (!port->currentId)
            return;
        value = port->sequenceStep;
        port->communicationState = one;
        value++;
        port->sequenceStep = value;
        return;
        }
    initial:
        value = port->sequenceStep;
        port->infoRecordIndex = 0;
    increment:
        value++;
        port->sequenceStep = value;
        return;
    terminal:
        value = 255;
        port->sequenceStep = value;
        return;
    advance:
        if (port->fn_18)
            value = port->fn_18(port);
        else
            value = _padRecvAtLoadInfo(port);
        port->sequenceStep += value;
    }
}

/* Psy-Q LIBPAD/PADSEQD.OBJ _dirFailAuto. */
void _dirFailAuto(CardObj *port) {
    port->frameCounter++;
    switch (port->sequenceStep) {
    case 0:
        break;
    case 1:
        if (port->retryCount < 2)
            port->retryCount++;
        else {
            port->communicationState = 2;
            port->sequenceStep = 255;
        }
        return;
    default:
        if (port->retryCount < 4) {
            port->retryCount++;
            return;
        }
        if (port->communicationState)
            g_MemCardObjResetFn(port);
    }
    if (*port->response_3c != 0xf3) {
        port->output_30[0] = 255;
        port->output_30[1] = 0;
        port->currentId = 0;
    }
}

int CardObj_IsTransferActive(CardObj *obj) {
    int value;
    int field;

    if (obj->currentExtendedId != 0) {
        value = 0xFF;
        field = obj->sequenceStep;
        if (field == value) {
            goto ret_zero;
        }
    }

    return 1;

ret_zero:
    return 0;
}
