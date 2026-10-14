#include "pe1/geom_state.h"
#include "pe1/gte.h"
#include "pe1/render_camera.h"
#include "pe1/render_lighting.h"
#include "pe1/field_movement.h"
#include "pe1/render_object.h"

GeomScrollRecord g_GeomScroll;

/* Resets the view state at the start of a room draw: render flags, the
 * scroll and camera words, the light colour, the mesh group and depth, and
 * points the view at the given matrix and projection distance. */
int Render_InitViewState(void *matrix, void *screen) {
    g_GeomScroll.words.flags = 0x70;
    g_GeomScroll.state.position.meshTintLevel = 0x60;
    g_GeomScroll.state.position.focusHeight = 0x180;
    g_RoomLightTint.b = 0xFF;
    g_RoomLightTint.g = 0xFF;
    g_RoomLightTint.r = 0xFF;
    g_GeomScroll.words.position = 0;
    g_GeomScroll.words.saved = 0;
    g_GeomScroll.words.screenOffset = 0;
    g_GeomScroll.words.start = 0;
    g_GeomScroll.words.target = 0;
    g_GeomScroll.words.timer = 0;
    g_GeomScroll.words.viewMatrix = matrix;
    g_GeomScroll.words.projectionDistance = screen;
    g_GeomScroll.words.savedBoundsX = 0;
    g_GeomScroll.words.savedBoundsY = 0;
    g_GeomScroll.words.cameraOffset = 0;
    g_GeomScroll.state.position.groupSelect = 0;
    g_GeomScroll.state.position.baseAngleAnalog = 0;
    g_GeomScroll.state.position.baseAngleDigital = 0;
    g_GeomScroll.state.position.otZ = 0;
    g_GeomScroll.words.origin = 0;
    return 0;
}

/* Clamps the camera position into the selected group's viewport. */
s32 Render_ClampCameraPosition(s32 x, s32 y)
{
    u16 cameraX = x;

    if (g_GeomScroll.state.flags & 0x40) {
        GeomStateAddress address, base;
        CameraViewport *view;

        GEOM_STATE_OFFSET(address, base, entry_offset_1C, 0);
        view = &address.viewport[g_GeomScroll.state.position.groupSelect];
        if ((s16)cameraX < view->minX)
            g_GeomScroll.state.position.x = view->minX;
        else if (view->maxX < (s16)cameraX)
            g_GeomScroll.state.position.x = view->maxX;
        else
            g_GeomScroll.state.position.x = x;
        if ((s16)y < view->minY) {
            g_GeomScroll.state.position.y = view->minY;
            return 0;
        }
        if (view->maxY < (s16)y) {
            g_GeomScroll.state.position.y = view->maxY;
            return 0;
        }
        g_GeomScroll.state.position.y = y;
    }
    return 0;
}
/* Saves the selected viewport's bounds and opens them to the full range. */
int Render_SaveAndOpenBounds(void) {
    GeomStateAddress address, base;
    CameraViewport *view;

    GEOM_STATE_OFFSET(address, base, entry_offset_1C, 0);
    view = &address.viewport[g_GeomScroll.state.position.groupSelect];
    g_GeomScroll.state.position.boundsMinX = view->minX;
    g_GeomScroll.state.position.boundsMaxX = view->maxX;
    g_GeomScroll.state.position.boundsMinY = view->minY;
    g_GeomScroll.state.position.boundsMaxY = view->maxY;
    view->minX = -0x8000;
    view->maxX = 0x7FFF;
    view->minY = -0x8000;
    view->maxY = 0x7FFF;
    return 0;
}

/* Restores the bounds saved by Render_SaveAndOpenBounds. */
int Render_RestoreBounds(void) {
    GeomStateAddress address, base;
    CameraViewport *view;

    GEOM_STATE_OFFSET(address, base, entry_offset_1C, 0);
    view = &address.viewport[g_GeomScroll.state.position.groupSelect];
    view->minX = g_GeomScroll.state.position.boundsMinX;
    view->maxX = g_GeomScroll.state.position.boundsMaxX;
    view->minY = g_GeomScroll.state.position.boundsMinY;
    view->maxY = g_GeomScroll.state.position.boundsMaxY;
    return 0;
}
/* Follow the projected world position within the selected viewport. */

int Render_SetViewport(const s32 *position) {
    struct {
        s16 point[4];
        s16 projected[2];
        unsigned reserved[10];
    } local = {0};
    GeomScrollState *camera = &g_GeomScroll.state;
    GeomStateAddress geometry, viewAddress;
    CameraViewport *view;
    unsigned int cx;
    GeomStateAddress offset_geometry;
    int flags = camera->flags;
    int screen_x;
    int screen_y;
    int sum_x;
    int sum_y;
    int view_x;
    int view_y;
    int half_width;
    int half_height;
    int axisValue;
    int viewportValue;
    unsigned raw_width;
    int target_x;
    int target_y;
    int y_bound;
    int selected_x;

    if (flags & 7)
        return -23;
    cx = 160;
    if (!(flags & 64))
        return -24;
    {
        int value = position[0] >> 16;

        local.point[0] = value;
    }
    {
        int value = position[1] >> 16;

        local.point[1] = value;
    }
    {
        int value = position[2] >> 16;

        local.point[2] = value;
    }
    {
        unsigned int cy = 112;
        gte_SetGeomOffset(cx, cy);
    }
    {
        u32 **address = &camera->position.matrixWords;
        {
            const GteMatrixWords *words;
            words = (const GteMatrixWords *)*address;
            gte_ldrotmatrix(words);
            gte_ldtransmatrix(words);
        }
    }
    {
        int distance = *g_GeomScroll.state.position.projectionDistance;
        gte_ctc2_26(distance);
    }
    gte_ldv0(local.point);
    gte_rtps();
    gte_stsxy2(local.projected);
    screen_x = local.projected[0];
    screen_y = local.projected[1];
    if (camera->flags & 128) {
        g_GeomScroll.state.position.cameraOffsetX = screen_x;
        g_GeomScroll.state.position.cameraOffsetY = screen_y;
    }
    geometry.state = g_GeomState;
    sum_x = g_GeomScroll.state.position.cameraOffsetX + screen_x;
    screen_x = sum_x / 2;
    g_GeomScroll.state.position.cameraOffsetX = screen_x;
    sum_y = g_GeomScroll.state.position.cameraOffsetY + screen_y;
    sum_y += (unsigned)sum_y >> 31;
    view_x = 320 - screen_x;

    offset_geometry.state = g_GeomState;

    screen_y = sum_y >> 1;
    g_GeomScroll.state.position.cameraOffsetY = screen_y;
    viewAddress.word = geometry.word + offset_geometry.state->entry_offset_1C;
    view = viewAddress.viewport;
    view += g_GeomScroll.state.position.groupSelect;
    viewportValue = 224;

    raw_width = view->width;
    view_y = viewportValue - screen_y;
    raw_width <<= 16;
    viewportValue = (int)raw_width >> 16;
    viewportValue += raw_width >> 31;
    half_width = viewportValue >> 1;
    axisValue = half_width - 160;

    viewportValue = view->height;
    target_x = axisValue + screen_x;
    viewportValue = (unsigned)viewportValue << 16;
    axisValue = viewportValue >> 16;
    axisValue += (unsigned)viewportValue >> 31;
    half_height = axisValue >> 1;
    target_y = half_height + (screen_y - 112);
    viewportValue = view->minX;
    if (target_x < viewportValue ||
        (viewportValue = view->maxX, viewportValue < target_x)) {
        axisValue = viewportValue - 160;
        view_x = half_width - axisValue;
    }
    y_bound = view->minY;
    if (target_y < y_bound ||
        (y_bound = view->maxY, y_bound < target_y)) {
        axisValue = view->height;
        viewportValue = (s16)axisValue / 2;
        axisValue = y_bound - 112;
        view_y = viewportValue - axisValue;
    }
    g_GeomScroll.state.position.screenOffsetX = view_x;
    g_GeomScroll.state.position.screenOffsetY = view_y;
    selected_x = target_x;
    if (g_GeomScroll.state.flags & 64) {
        int signed_y;
        s16 chosen_bound;
        int upper_y;
        geometry.state = g_GeomState;
        viewAddress.word = geometry.word + geometry.state->entry_offset_1C;
        view = viewAddress.viewport;
        view += g_GeomScroll.state.position.groupSelect;
        viewportValue = (s16)target_x;
        axisValue = view->minX;
        chosen_bound = axisValue;
        if (viewportValue < axisValue) {
            signed_y = (unsigned)target_y << 16;
            g_GeomScroll.state.position.x = chosen_bound;
        } else {
            axisValue = view->maxX;
            chosen_bound = axisValue;
            if (axisValue < viewportValue) {
                signed_y = (unsigned)target_y << 16;
                g_GeomScroll.state.position.x = chosen_bound;
            } else {
                signed_y = (unsigned)target_y << 16;
                g_GeomScroll.state.position.x = selected_x;
            }
        }
        signed_y >>= 16;
        axisValue = view->minY;
        chosen_bound = axisValue;
        if (signed_y < axisValue) {
            g_GeomScroll.state.position.y = chosen_bound;
            return 0;
        }
        axisValue = view->maxY;
        upper_y = axisValue;
        if (axisValue < signed_y) {
            g_GeomScroll.state.position.y = upper_y;
            return 0;
        }
        g_GeomScroll.state.position.y = target_y;
    }
    return 0;
}
/* Moves the GTE screen offset to the current screen offset. */
int Render_SetGteScreenOffset(void) {
    gte_SetGeomOffset(g_GeomScroll.state.position.screenOffsetX,
                      g_GeomScroll.state.position.screenOffsetY);
    return 0;
}

/* Restores the GTE screen offset to the screen centre. */
int Render_ResetGteScreenOffset(void) {
    gte_SetGeomOffset(160, 112);
    return 0;
}

/* Starts a camera scroll from the current position to (x, y) over
 * `duration` frames; mode 8 selects the smoothed step. */
int Render_SetScrollMode(int x, int y, int duration, int mode) {
    s32 *flags = &g_GeomScroll.state.flags;
    int value = *flags;
    u32 position;
    int bits;

    if (!(value & 0x40))
        return -19;
    position = g_GeomScroll.words.position;
    g_GeomScroll.state.position.elapsed = 1;
    g_GeomScroll.state.position.targetX = x;
    g_GeomScroll.state.position.targetY = y;
    g_GeomScroll.state.position.duration = duration;
    g_GeomScroll.words.start = position;
    position = value & ~0xF;
    if (mode != 8)
        bits = position | 1;
    else
        bits = position | 9;
    *flags = bits;
    return 0;
}

/* Advance the camera transition, then center the selected viewport. */
int Render_StepCameraTransition(void)
{
    unsigned int flags = g_GeomScroll.state.flags;
    int mode, smooth;
    u16 dx, dy, duration;
    if (!(flags & 0x40))
        return -20;
    mode = flags & 7;
    smooth = flags & 8;
    if (mode != 1 && mode != 2 && mode != 3)
        return 0;
    {
        u16 targetX = g_GeomScroll.state.position.targetX, x = g_GeomScroll.state.position.startX;
        u16 targetY = g_GeomScroll.state.position.targetY, y = g_GeomScroll.state.position.startY;
        u16 time = g_GeomScroll.state.position.elapsed;
        duration = g_GeomScroll.state.position.duration;
        dx = targetX - x;
        dy = targetY - y;
        if (!smooth) {
            int movedX = (short)dx * (short)time / (short)duration;
            int movedY = (short)dy * (short)time / (short)duration;
            g_GeomScroll.state.position.x = x + movedX;
            g_GeomScroll.state.position.y = y + movedY;
        } else {
            int weight = rcos((short)time * 2048 / (short)duration + 2048) + 4096;
            int movedX = (short)dx * weight / 8192;
            int movedY = (short)dy * weight / 8192;
            g_GeomScroll.state.position.x = g_GeomScroll.state.position.startX + movedX;
            g_GeomScroll.state.position.y = g_GeomScroll.state.position.startY + movedY;
        }
    }
    {
        GeomStateAddress views, base;
        CameraViewport *view;
        GEOM_STATE_OFFSET(views, base, entry_offset_1C, 0);
        view = &views.viewport[g_GeomScroll.state.position.groupSelect];
        {
            unsigned int offset = 160 - (g_GeomScroll.state.position.x - (short)view->width / 2);
            g_GeomScroll.state.position.screenOffsetX = offset;
        }
        {
            unsigned int offset = 112 - (g_GeomScroll.state.position.y - (short)view->height / 2);
            g_GeomScroll.state.position.screenOffsetY = offset;
        }
    }
    if (mode == 1)
        g_GeomScroll.state.flags = (g_GeomScroll.state.flags & ~7) | 2;
    {
        u16 time = g_GeomScroll.state.position.elapsed + 1;
        g_GeomScroll.state.position.elapsed = time;
        if ((short)time > (short)duration) {
            unsigned int completed = g_GeomScroll.state.flags;
            if ((completed & 7) == 2)
                g_GeomScroll.state.flags = (completed & ~7) | 0x84;
            else
                g_GeomScroll.state.flags = (completed & ~7) | 0x80;
        }
    }
    return 0;
}

/* Project a world-space point and start a clamped camera transition. */
/* Starts a camera scroll towards the projection of a world position,
 * clamped to the selected viewport (duration -1 = 30 frames, mode -1 =
 * plain step). */
int Render_UpdateScrollPosition(void *positionArg, int duration, int mode)
{
    int *position = positionArg;
    GteShortVector vector = {0};
    s16 projected[2];
    GeomScrollState *camera = &g_GeomScroll.state;
    CameraViewport *view;
    int x, y;

    if (!(camera->flags & 64))
        return -21;
    g_GeomScroll.state.position.startX = g_GeomScroll.state.position.x;
    g_GeomScroll.state.position.startY = (u16)g_GeomScroll.state.position.y;
    vector.x = position[0] >> 16;
    vector.y = (position[1] >> 16) - (u16)g_GeomScroll.state.position.focusHeight;
    vector.z = position[2] >> 16;
    {
        /* Matching debt: see Render_PrepareFrame; this volatile * input is the only form found that keeps the 24-byte frame. */
        volatile unsigned short cx = 160;

        gte_SetGeomOffset(cx, 112);
    }
    {
        GteMatrix **slot = &g_GeomScroll.words.viewMatrix;
        GteMatrix *matrix;

        matrix = *slot;
        gte_ldrotmatrix(matrix);
        gte_ldtransmatrix(matrix);
    }
    gte_ldv0(&vector);
    gte_rtps();
    gte_stsxy2(projected);
    {
        GeomState *state = g_GeomState;
        GeomStateAddress table;

        table.state = g_GeomState;
        table.word += state->entry_offset_1C;
        view = &table.viewport[g_GeomScroll.state.position.groupSelect];
    }
    {
        u16 width = view->width,height = view->height;
        int halfX = (short)width/2-160,halfY = (short)height/2-112;
        x = halfX+projected[0];
        y = halfY+projected[1];
    }
    if (x < view->minX)
        x = view->minX;
    else if (x > view->maxX)
        x = view->maxX;
    if (y < view->minY)
        y = view->minY;
    else if (y > view->maxY)
        y = view->maxY;
    g_GeomScroll.state.position.targetX = x;
    g_GeomScroll.state.position.targetY = y;
    if (duration == -1)
        g_GeomScroll.state.position.duration = 30;
    else
        g_GeomScroll.state.position.duration = duration;
    g_GeomScroll.state.position.elapsed = 1;
    if (mode == -1)
        g_GeomScroll.state.flags = (g_GeomScroll.state.flags & ~7) | 3;
    else {
        unsigned int flags = g_GeomScroll.state.flags & ~15;
        unsigned int bits = mode | 3;
        g_GeomScroll.state.flags = flags | bits;
    }
    return 0;
}

